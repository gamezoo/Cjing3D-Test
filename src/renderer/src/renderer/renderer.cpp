#include "renderer.h"
#include "renderScene.h"
#include "renderImage.h"
#include "renderGraph.h"
#include "textureHelper.h"
#include "resource\resourceManager.h"
#include "core\platform\platform.h"
#include "core\scene\universe.h"
#include "core\helper\profiler.h"
#include "core\helper\enumTraits.h"
#include "core\scene\reflection.h"

namespace Cjing3D
{
namespace Renderer
{
	// render instance for DrawInstanced
	struct RenderInstance
	{
		F32x4 mMat0;
		F32x4 mMat1;
		F32x4 mMat2;
		I32x4 mUserdata;

		void Setup(const F32x4x4& mat, const F32x4 color = F32x4(1.0f, 1.0f, 1.0f, 1.0f))
		{
			mMat0 = mat.GetColumn(0);
			mMat1 = mat.GetColumn(1);
			mMat2 = mat.GetColumn(2);
			mUserdata = I32x4(0, 0, 0, 0);
		}
	};

	// Object render batch  for object rendering
	struct ObjectRenderBatch
	{
		U64 mHash = 0;
		U32 mObjectIndex = 0;

		void Setup(U32 meshIndex, U32 objectIndex, F32 distance)
		{
			mHash = ((U64)distance) << 32 | (U64)meshIndex;
			mObjectIndex = objectIndex;
		}

		U32 GetMeshIndex()
		{
			return (U32)(mHash & 0xFFFFFFFF);
		}
	};

	struct ObjectRenderQueue
	{
		enum SortMethod
		{
			FrontToBack,
			BackToFront,
		};

		DynamicArray<ObjectRenderBatch*> mRenderBatches;

		bool IsEmpty()const {
			return mRenderBatches.empty();
		}
		
		void AddBatch(ObjectRenderBatch* batch) {
			mRenderBatches.push(batch);
		}

		void Sort(SortMethod sortMethod)
		{
			std::sort(mRenderBatches.begin(), mRenderBatches.end(), 
				[sortMethod](const ObjectRenderBatch* a, const ObjectRenderBatch* b) {
					return sortMethod == FrontToBack ? a->mHash < b->mHash : a->mHash > b->mHash;
				});
		}
	};

	struct InstanceHandler
	{
	public:
		using CheckConditionFunc = std::function<bool(U32, U32, RenderScene&)>;
		using ProcessInstanceFunc = std::function<void(U32, RenderInstance&)>;

		CheckConditionFunc checkCondition_ = nullptr;
		ProcessInstanceFunc processInstance_ = nullptr;
	};

	//////////////////////////////////////////////////////////////////////////
	// Impl
	//////////////////////////////////////////////////////////////////////////
	class RendererImpl
	{
	public:
		RendererImpl();
		~RendererImpl();

		void PreLoadShaders();
		void PreLoadBuffers();

		void BindCommonResources(ShaderBindingContext& context);
		void BindConstantBuffers(ShaderBindingContext& context, GPU::SHADERSTAGES stage);

		void ProcessRenderQueue(
			ObjectRenderQueue& queue, 
			RENDERPASS renderPass, 
			RENDERTYPE renderType,
			const CullingResult& cullResult, 
			RenderGraphResources& resources, 
			GPU::CommandList& cmd,
			ShaderBindingContext& shaderContext, 
			I32 handlerCount = 1, 
			InstanceHandler* instanceHandler = nullptr);

		ShaderTechnique GetObjectTech(RENDERPASS renderPass, BLENDMODE blendMode);
		ShaderTechnique GetObjectTech(Shader& shader, RENDERPASS renderPass, BLENDMODE blendMode);

	public:
		Concurrency::RWLock mLock;
		RenderScene* mRenderScene = nullptr;

		StaticArray<ShaderRef, SHADERTYPE_COUNT> mShaders;
		StaticArray<GPU::ResHandle, CBTYPE_COUNT> mConstantBuffers;
		DynamicArray<GPU::ResHandle> mSamplerStates;
	};

	RendererImpl::RendererImpl()
	{
	}

	RendererImpl::~RendererImpl()
	{
		for (int i = 0; i < SHADERTYPE_COUNT; i++) {
			mShaders[i].Reset();
		}

		// release gpu handles
		for (const auto& handle : mConstantBuffers)
		{
			if (handle != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(handle);
			}
		}

		for (const auto& handle : mSamplerStates)
		{
			if (handle != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(handle);
			}
		}

		mRenderScene = nullptr;
	}

	void RendererImpl::PreLoadBuffers()
	{
		GPU::BufferDesc desc = {};
		desc.mByteWidth = sizeof(FrameCB);
		desc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
		mConstantBuffers[CBTYPE_FRAME] = GPU::CreateBuffer(&desc, nullptr, "FrameCB");

		desc.mByteWidth = sizeof(CameraCB);
		desc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
		mConstantBuffers[CBTYPE_CAMERA] = GPU::CreateBuffer(&desc, nullptr, "CameraCB");
	}

	void RendererImpl::BindCommonResources(ShaderBindingContext& context)
	{
		for (int i = 0; i < GPU::SHADERSTAGES_COUNT; i++) {
			BindConstantBuffers(context, (GPU::SHADERSTAGES)i);
		}
	}

	void RendererImpl::BindConstantBuffers(ShaderBindingContext& context, GPU::SHADERSTAGES stage)
	{
		ShaderBindingSet bindingSet = Shader::CreateGlobalBindingSet("CommonBindings");
		bindingSet.Set("gFrameCB", GPU::Binding::ConstantBuffer(mConstantBuffers[CBTYPE_FRAME], stage));
		bindingSet.Set("gCameraCB", GPU::Binding::ConstantBuffer(mConstantBuffers[CBTYPE_CAMERA], stage));
	}

	void RendererImpl::PreLoadShaders()
	{
		// clear current resource jobs
		ResourceManager::WaitAll();

		mShaders[SHADERTYPE_MAIN]  = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/main_pipeline.jsf"));
		mShaders[SHADERTYPE_IMAGE] = ShaderRef(ResourceManager::LoadResource<Shader>("shaders/render_image.jsf"));

		// wait for all shaders
		ResourceManager::WaitAll();
	}

	void RendererImpl::ProcessRenderQueue(
		ObjectRenderQueue& queue,
		RENDERPASS renderPass, 
		RENDERTYPE renderType, 
		const CullingResult& cullResult, 
		RenderGraphResources& resources, 
		GPU::CommandList& cmd, 
		ShaderBindingContext& shaderContext,
		I32 handlerCount, 
		InstanceHandler* instanceHandler)
	{
		if (queue.IsEmpty()) {
			return;
		}
		cmd.EventBegin("ProcessRenderQueue");

		RenderScene& scene = *mRenderScene;
		auto transforms = scene.GetUniverse().GetComponents<Transform>(ECS::SceneReflection::GetComponentType("Transform"));

		// RenderInstance:
		// * worldMatrix(mMat0, mMat1, mMat2);
		// * I32x4 mUserdata

		// allocate all intances
		I32 instanceSize = queue.mRenderBatches.size() * sizeof(RenderInstance);
		GPU::GPUAllocation instanceAllocation = cmd.GPUAlloc(instanceSize);

		// InstancedBatch combines ObjectRenderBatch by the same meshIndex
		struct InstancedBatch
		{
			U32 mMeshIndex = 0;
			U32 mInstanceCount = 0;
			U32 mInstanceOffset = 0;
		};
		DynamicArray<InstancedBatch*> instancedBatches;

		// 对于相邻且MeshIndex相同的ObjectRenderBatch会合并为一个InstancedBatch
		I32 totalInstanceCount = 0;
		I32 prevMeshIndex = ~0;
		for (ObjectRenderBatch* renderBatch : queue.mRenderBatches)
		{
			const I32 objectIndex = renderBatch->mObjectIndex;
			const I32 meshIndex = renderBatch->GetMeshIndex();
			if (meshIndex != prevMeshIndex)
			{
				prevMeshIndex = meshIndex;

				InstancedBatch* instancedBatch = cmd.Alloc<InstancedBatch>();
				instancedBatch->mMeshIndex = meshIndex;
				instancedBatch->mInstanceCount = 0;
				instancedBatch->mInstanceOffset = instanceAllocation.mOffset + totalInstanceCount * sizeof(RenderInstance);

				instancedBatches.push(instancedBatch);
			}

			// 创建一个新的RenderInstance并添加到当前InstancedBatch中
			InstancedBatch& currentBatch = *instancedBatches.back();
			const ObjectComponent* object = scene.mObjects->GetComponentByIndex(objectIndex);
			if (object == nullptr) {
				continue;
			}

			F32x4x4 worldMatrix = IDENTITY_MATRIX;
			Transform* transform = object->mTransformIndex >= 0 ? transforms->GetComponentByIndex(object->mTransformIndex) : nullptr;
			if (transform != nullptr) {
				worldMatrix = transform->mWorld;
			}

			for (U32 handleIndex = 0; handleIndex < handlerCount; handleIndex++)
			{
				if ( instanceHandler != nullptr &&
					 instanceHandler->checkCondition_ != nullptr &&
					!instanceHandler->checkCondition_(handleIndex, objectIndex, scene)) {
					continue;
				}

				// setup renderInstance from worldMatrix and object color
				RenderInstance& renderInstance = ((RenderInstance*)instanceAllocation.mData)[totalInstanceCount];
				renderInstance.Setup(worldMatrix, object->mColor);

				if (instanceHandler != nullptr &&
					instanceHandler->processInstance_ != nullptr) {
					instanceHandler->processInstance_(handleIndex, renderInstance);
				}

				currentBatch.mInstanceCount++;
				totalInstanceCount++;
			}
		}

		// process instanced batches
		for (InstancedBatch* batch : instancedBatches)
		{
			const MeshComponent* mesh = scene.mMeshes->GetComponentByIndex(batch->mMeshIndex); 
			if (!mesh) {
				continue;
			}

			cmd.BindIndexBuffer(GPU::Binding::IndexBuffer(mesh->mIndexBuffer, 0), mesh->GetIndexFormat());

			// bind vertex buffer
			DynamicArray<GPU::BindingBuffer> buffers;
			buffers.push(GPU::Binding::VertexBuffer(mesh->mVertexBufferPos,   0, sizeof(MeshComponent::VertexPos)));
			buffers.push(GPU::Binding::VertexBuffer(mesh->mVertexBufferTex,   0, sizeof(MeshComponent::VertexTex)));
			buffers.push(GPU::Binding::VertexBuffer(mesh->mVertexBufferColor, 0, sizeof(MeshComponent::VertexColor)));
			buffers.push(GPU::Binding::VertexBuffer(instanceAllocation.mBuffer, batch->mInstanceOffset, sizeof(RenderInstance)));
			cmd.BindVertexBuffer(Span(buffers.data(), buffers.size()), 0);

			// render mesh subsets
			for (const auto& subset : mesh->mSubsets)
			{
				if (subset.mIndexCount <= 0) {
					continue;
				}

				MaterialComponent* material = scene.mMaterials->GetComponent(subset.mMaterialID);
				if (!material) {
					continue;
				}

				// check render enable
				bool isRenderable = true;

				if (!isRenderable) {
					continue;
				}

				// get target shader technique
				ShaderTechnique tech;
				if (material->mUseCustomShader && material->mMaterial)
				{
					// custom shader
					tech = GetObjectTech(*material->mMaterial->GetShader(), renderPass, material->GetBlendMode());
				}
				else
				{
					tech = GetObjectTech(renderPass, material->GetBlendMode());
				}

				if (!tech || tech.GetPipelineState() == GPU::ResHandle::INVALID_HANDLE) {
					continue;
				}

				// bind material constant buffer
				ShaderBindingSet bindingSet = Shader::CreateGlobalBindingSet("MaterialBindings");
				bindingSet.Set("constBuffer_Material", GPU::Binding::ConstantBuffer(material->mConstantBuffer, GPU::SHADERSTAGES_VS));
				bindingSet.Set("constBuffer_Material", GPU::Binding::ConstantBuffer(material->mConstantBuffer, GPU::SHADERSTAGES_PS));

				// bind material textures
				bindingSet.Set("texture_BaseColorMap", GPU::Binding::Texture(material->GetTexture(Material::BaseColorMap), GPU::SHADERSTAGES_PS));
				bindingSet.Set("texture_NormalMap",    GPU::Binding::Texture(material->GetTexture(Material::NormalMap), GPU::SHADERSTAGES_PS));
				bindingSet.Set("texture_SurfaceMap",   GPU::Binding::Texture(material->GetTexture(Material::SurfaceMap), GPU::SHADERSTAGES_PS));

				if (shaderContext.Bind(tech, bindingSet))
				{
					cmd.BindPipelineState(tech.GetPipelineState());
					cmd.DrawIndexedInstanced(subset.mIndexCount, batch->mInstanceCount, subset.mIndexOffset, 0, 0);
				}
			}
		}

		cmd.Free(instancedBatches.size() * sizeof(InstancedBatch));
		cmd.EventEnd();
	}

	ShaderTechnique RendererImpl::GetObjectTech(RENDERPASS renderPass, BLENDMODE blendMode)
	{
		auto shader = GetShader(SHADERTYPE_MAIN);
		return GetObjectTech(*shader, renderPass, blendMode);
	}

	ShaderTechnique RendererImpl::GetObjectTech(Shader& shader, RENDERPASS renderPass, BLENDMODE blendMode)
	{
		ShaderTechHasher hasher;
		hasher.mRenderPass = renderPass;
		hasher.mBlendMode = blendMode;

		ShaderTechniqueDesc desc = {};
		desc.mPrimitiveTopology = GPU::TRIANGLESTRIP;
		return std::move(shader.CreateTechnique(hasher, desc));
	}

	//////////////////////////////////////////////////////////////////////////
	// Function
	//////////////////////////////////////////////////////////////////////////
	RendererImpl* mImpl = nullptr;

	void Initialize(GPU::GPUSetupParams params, bool loadShaders)
	{
		if (IsInitialized()) {
			return;
		}

		// initialize gpu 
		GPU::Initialize(params);

		// register render res factories
		Shader::RegisterFactory();
		Texture::RegisterFactory();
		Model::RegisterFactory();
		Material::RegisterFactory();

		// initialize impl
		mImpl = CJING_NEW(RendererImpl);
		mImpl->PreLoadBuffers();

		// editor may load shaders lazy
		if (loadShaders) {
			mImpl->PreLoadShaders();
		}

		// initialize renderImage
		RenderImage::Initialize();

		// initialize texture helper
		TextureHelper::Initialize();
	}

	bool IsInitialized()
	{
		return mImpl != nullptr;
	}

	void Uninitialize()
	{
		if (!IsInitialized()) {
			return;                                                                                                                                                                                                                                                                                            
		}

		// uninitialize texture helper
		TextureHelper::Uninitialize();

		// uninitialize impl
		CJING_SAFE_DELETE(mImpl);

		// unregister render res factories
		ResourceManager::ProcessReleasedResources();
		Material::UnregisterFactory();
		Model::UnregisterFactory();
		Texture::UnregisterFactory();
		Shader::UnregisterFactory();

		// uninitialize gpu
		GPU::Uninitialize();
	}

	void InitRenderScene(Engine& engine, Universe& universe)
	{
		UniquePtr<RenderScene> renderScene = CJING_MAKE_UNIQUE<RenderScene>(engine, universe);
		mImpl->mRenderScene = renderScene.Get();
		universe.AddScene(std::move(renderScene));
	}

	void Update(CullingResult& visibility, FrameCB& frameCB, F32 deltaTime)
	{
		auto screenSize = GPU::GetScreenSize();
		frameCB.gFrameScreenSize = screenSize;
		frameCB.gFrameScreenSizeRCP = F32x2(1.0f / screenSize[0], 1.0f / screenSize[1]);
	}

	void PresentBegin(GPU::CommandList& cmd)
	{
		GPU::PresentBegin(cmd);
	}

	void PresentEnd()
	{
		GPU::PresentEnd();
	}

	void EndFrame()
	{
		GPU::EndFrame();
	}

	void DrawScene(RENDERPASS renderPass, RENDERTYPE renderType, const CullingResult& cullResult, RenderGraphResources& resources, GPU::CommandList& cmd)
	{
		if (!IsInitialized()) {
			return;
		}

		if (!mImpl->mRenderScene) {
			return;
		}

		auto& scene = *mImpl->mRenderScene;
		// bind common resources (cbs, samplers)
		ShaderBindingContext context(cmd);
		mImpl->BindCommonResources(context);

		// setup render queue
		ObjectRenderQueue queue;
		for (int i = 0; i < cullResult.mObjectCount; i++)
		{
			I32 index = cullResult.mCulledObjects[i];
			ObjectComponent* object = scene.mObjects->GetComponentByIndex(index);
			if (object == nullptr || !object->IsRenderable()) {
				continue;
			}

			F32 distance = Distance(cullResult.mViewport->mEye, object->mCenter);
			ObjectRenderBatch* batch = cmd.Alloc<ObjectRenderBatch>();
			batch->Setup(scene.mMeshes->GetEntityIndex(object->mMeshID), index, distance);
		}

		if (!queue.IsEmpty())
		{
			// 对于Transparent需要从后往前排序，来实现blending，而对于object则需要从前往后，根据depthCmp来减少over draw
			queue.Sort(renderType != RENDERTYPE_TRANSPARENT ? ObjectRenderQueue::FrontToBack : ObjectRenderQueue::BackToFront);
			mImpl->ProcessRenderQueue(queue, renderPass, renderType, cullResult, resources, cmd, context);
		}
	}

	GPU::ResHandle GetConstantBuffer(CBTYPE type)
	{
		Debug::CheckAssertion(IsInitialized());
		return mImpl->mConstantBuffers[(U32)type];
	}

	ShaderRef GetShader(SHADERTYPE type)
	{
		Debug::CheckAssertion(IsInitialized());
		if (mImpl->mShaders[(U32)type] == nullptr) {
			Logger::Warning("Failed to get shader:(shaderType) %d", (I32)type);
		}
		return mImpl->mShaders[(U32)type];
	}

	ShaderRef LoadShader(const char* path, bool waitFor)
	{
		auto shader = ShaderRef(ResourceManager::LoadResource<Shader>(path));
		if (waitFor) {
			ResourceManager::WaitForResource(shader);
		}
		return shader;
	}

	void LoadAllShaders()
	{
		Debug::CheckAssertion(IsInitialized());
		mImpl->PreLoadShaders();
	}

	RenderScene* GetRenderScene()
	{
		return mImpl->mRenderScene;
	}

	void AddStaticSampler(const GPU::ResHandle& handle, I32 slot)
	{
		// add static global sampler
		Concurrency::ScopedReadLock lock(mImpl->mLock);
		if (!handle) {
			return;
		}
		mImpl->mSamplerStates.push(handle);
		
		GPU::StaticSampler sam;
		sam.mSampler = handle;
		sam.mSlot = slot;
		GPU::AddStaticSampler(sam);
	}

	void UpdateViewCulling(CullingResult& cullingResult, Viewport& viewport, I32 cullingFlag)
	{
		Profiler::BeginCPUBlock("Frustum Culling");

		cullingResult.Clear();

		auto scene = GetRenderScene();
		if (scene != nullptr) {
			scene->GetCullingResult(cullingResult, viewport.mFrustum, cullingFlag);
		}

		Profiler::EndCPUBlock();
	}

	void UpdateCameraCB(const Viewport& viewport, CameraCB& cameraCB)
	{
		cameraCB.gCameraVP = XMStore<F32x4x4>(viewport.GetViewProjectionMatrix());
		cameraCB.gCameraView = XMStore<F32x4x4>(viewport.GetViewMatrix());
		cameraCB.gCameraProj = XMStore<F32x4x4>(viewport.GetProjectionMatrix());
		cameraCB.gCameraInvV = XMStore<F32x4x4>(viewport.GetInvViewMatrix());
		cameraCB.gCameraInvP = XMStore<F32x4x4>(viewport.GetInvProjectionMatrix());
		cameraCB.gCameraInvVP = XMStore<F32x4x4>(viewport.GetInvViewProjectionMatrix());

		cameraCB.gCameraPos = viewport.mAt;
		cameraCB.gCameraNearZ = viewport.mNear;
		cameraCB.gCameraFarZ = viewport.mFar;
		cameraCB.gCameraInvNearZ = (1.0f / std::max(0.00001f, cameraCB.gCameraNearZ));
		cameraCB.gCameraInvFarZ = (1.0f / std::max(0.00001f, cameraCB.gCameraFarZ));
	}
}
}