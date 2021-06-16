#include "renderer.h"
#include "renderScene.h"
#include "renderImage.h"
#include "textureHelper.h"
#include "renderGraph\renderGraph.h"
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

		void BindCommonResources(RenderGraphResources& resources, ShaderBindingContext& context);
		void BindConstantBuffers(RenderGraphResources& resources, ShaderBindingContext& context, GPU::SHADERSTAGES stage);

		void ProcessMeshRenderQueue(
			ObjectRenderQueue& queue, 
			RENDERPASS renderPass, 
			RENDERTYPE renderType,
			const Visibility& cullResult, 
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
		DynamicArray<GPU::ResHandle> mSamplerStates;
		F32 mResolutionScale = 1.0f;
		U32x2 mWindowSize = { 0u, 0u };
		U32 mShadowResolution2D = 1024;

		// Runtime render graph resources
		I32 mShadow2DCount = 8;
		RenderGraphResource mShadowTex2D; // DynamicArray<RenderGraphResource> mShadowTex2D;
		StaticArray<RenderGraphResource, CBTYPE_COUNT> mConstantBuffers;
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
		for (const auto& handle : mSamplerStates)
		{
			if (handle != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(handle);
			}
		}

		mRenderScene = nullptr;
	}

	void RendererImpl::BindCommonResources(RenderGraphResources& resources, ShaderBindingContext& context)
	{
		for (int i = 0; i < GPU::SHADERSTAGES_COUNT; i++) {
			BindConstantBuffers(resources, context, (GPU::SHADERSTAGES)i);
		}
	}

	void RendererImpl::BindConstantBuffers(RenderGraphResources& resources, ShaderBindingContext& context, GPU::SHADERSTAGES stage)
	{
		ShaderBindingSet bindingSet = Shader::CreateGlobalBindingSet("CommonBindings");
		
		bindingSet.Set("gFrameCB", GPU::Binding::ConstantBuffer(resources.GetBuffer(mConstantBuffers[CBTYPE_FRAME]), stage));
		bindingSet.Set("gCameraCB", GPU::Binding::ConstantBuffer(resources.GetBuffer(mConstantBuffers[CBTYPE_CAMERA]), stage));
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

	void RendererImpl::ProcessMeshRenderQueue(
		ObjectRenderQueue& queue,
		RENDERPASS renderPass, 
		RENDERTYPE renderType, 
		const Visibility& cullResult, 
		RenderGraphResources& resources, 
		GPU::CommandList& cmd, 
		ShaderBindingContext& shaderContext,
		I32 handlerCount, 
		InstanceHandler* instanceHandler)
	{
		if (queue.IsEmpty()) {
			return;
		}

		cmd.EventBegin("ProcessMeshRenderQueue");

		RenderScene& scene = *mRenderScene;
		auto transforms = scene.GetUniverse().GetComponents<Transform>(ECS::SceneReflection::GetComponentType("Transform"));

		// RenderInstance:
		// * worldMatrix(mMat0, mMat1, mMat2);
		// * I32x4 mUserdata

		U32 instanceDataSize = sizeof(RenderInstance);

		// allocate all intances
		I32 instanceSize = queue.mRenderBatches.size() * handlerCount * instanceDataSize;
		GPU::GPUAllocation instanceAllocation = cmd.GPUAlloc(instanceSize);

		// InstancedBatch combines ObjectRenderBatch by the same meshIndex
		struct InstancedBatch
		{
			U32 mMeshIndex = 0;
			U32 mInstanceCount = 0;
			U32 mInstanceOffset = 0;
		};
		InstancedBatch instancedBatch;
		instancedBatch.mMeshIndex = ~0;	// init mesh index unused

		// flush and render current render batch
		auto FlushRenderBatch = [&](const InstancedBatch& renderBatch)
		{
			if (renderBatch.mInstanceCount <= 0) {
				return;
			}

			const MeshComponent* mesh = scene.mMeshes->GetComponentByIndex(renderBatch.mMeshIndex);
			if (!mesh) {
				return;
			}

			cmd.BindIndexBuffer(GPU::Binding::IndexBuffer(mesh->mIndexBuffer, 0), mesh->GetIndexFormat());

			// bind vertex buffer
			DynamicArray<GPU::BindingBuffer> buffers;
			buffers.push(GPU::Binding::VertexBuffer(mesh->mVertexBufferPos, 0, sizeof(MeshComponent::VertexPos)));
			buffers.push(GPU::Binding::VertexBuffer(mesh->mVertexBufferTex, 0, sizeof(MeshComponent::VertexTex)));
			buffers.push(GPU::Binding::VertexBuffer(mesh->mVertexBufferColor, 0, sizeof(MeshComponent::VertexColor)));
			buffers.push(GPU::Binding::VertexBuffer(instanceAllocation.mBuffer, renderBatch.mInstanceOffset, sizeof(RenderInstance)));
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
				bindingSet.Set("texture_NormalMap", GPU::Binding::Texture(material->GetTexture(Material::NormalMap), GPU::SHADERSTAGES_PS));
				bindingSet.Set("texture_SurfaceMap", GPU::Binding::Texture(material->GetTexture(Material::SurfaceMap), GPU::SHADERSTAGES_PS));

				if (shaderContext.Bind(tech, bindingSet))
				{
					cmd.BindPipelineState(tech.GetPipelineState());
					cmd.DrawIndexedInstanced(subset.mIndexCount, renderBatch.mInstanceCount, subset.mIndexOffset, 0, 0);
				}
			}
		};

		// 对于相邻且MeshIndex相同的ObjectRenderBatch会合并为一个InstancedBatch
		I32 totalInstanceCount = 0;

		for (ObjectRenderBatch* renderBatch : queue.mRenderBatches)
		{
			const I32 objectIndex = renderBatch->mObjectIndex;
			const I32 meshIndex = renderBatch->GetMeshIndex();
			if (meshIndex != instancedBatch.mMeshIndex)
			{
				FlushRenderBatch(instancedBatch);

				instancedBatch.mMeshIndex = meshIndex;
				instancedBatch.mInstanceCount = 0;
				instancedBatch.mInstanceOffset = instanceAllocation.mOffset + totalInstanceCount * instanceDataSize;
			}

			// 创建一个新的RenderInstance并添加到当前InstancedBatch中
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

				instancedBatch.mInstanceCount++;
				totalInstanceCount++;
			}
		}

		FlushRenderBatch(instancedBatch);

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
		auto clientBounds = Platform::GetClientBounds(params.mWindow);
		mImpl->mWindowSize = {
			(U32)(clientBounds.mRight - clientBounds.mLeft),
			(U32)(clientBounds.mBottom - clientBounds.mTop) };

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

	void UpdatePerFrameData(Visibility& visibility, FrameCB& frameCB, F32 deltaTime, const U32x2& resolution)
	{
		frameCB.gFrameScreenSize = F32x2((F32)resolution.x(), (F32)resolution.y());
		frameCB.gFrameScreenSizeRCP = F32x2(1.0f / (F32)resolution.x(), 1.0f / (F32)resolution.y());
	}

	void EndFrame()
	{
		GPU::EndFrame();
	}

	void DrawScene(RENDERPASS renderPass, RENDERTYPE renderType, const Visibility& cullResult, RenderGraphResources& resources, GPU::CommandList& cmd)
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
		mImpl->BindCommonResources(resources, context);

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
			mImpl->ProcessMeshRenderQueue(queue, renderPass, renderType, cullResult, resources, cmd, context);
		}
	}

	RenderGraphResource GetConstantBuffer(CBTYPE type)
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

	void UpdateViewCulling(Visibility& cullingResult, Viewport& viewport, I32 cullingFlag)
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

	void SetupRenderData(RenderGraph& renderGraph, const FrameCB& frameCB, const Visibility& visibility, Viewport& viewport)
	{
		renderGraph.AddCallbackRenderPass("SetupRenderFrame",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder)->CallbackRenderPass::ExecuteFn {

				// create constant buffer
				GPU::BufferDesc desc = {};
				desc.mByteWidth = sizeof(FrameCB);
				desc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
				mImpl->mConstantBuffers[CBTYPE_FRAME] = builder.CreateBuffer("FrameCB", &desc);

				desc.mByteWidth = sizeof(CameraCB);
				desc.mBindFlags = GPU::BIND_CONSTANT_BUFFER;
				mImpl->mConstantBuffers[CBTYPE_CAMERA] = builder.CreateBuffer("CameraCB", &desc);

				// update camera buffer
				CameraCB cameraCB = {};
				Renderer::UpdateCameraCB(viewport, cameraCB);

				// Wait for this pass
				builder.WaitForThisPass();

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {
					// update constant buffer
					cmd.UpdateBuffer(resources.GetBuffer(mImpl->mConstantBuffers[CBTYPE_FRAME]), &frameCB, 0, sizeof(FrameCB));
					cmd.UpdateBuffer(resources.GetBuffer(mImpl->mConstantBuffers[CBTYPE_CAMERA]), &cameraCB, 0, sizeof(CameraCB));
				};
			});
	}

	void DrawShadowMaps(RenderGraph& renderGraph, const Visibility& visibility)
	{
		renderGraph.AddCallbackRenderPass("ShadowMaps",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder)->CallbackRenderPass::ExecuteFn {

				// create shadow map
				auto shadowTex2D = builder.GetTextureDesc(mImpl->mShadowTex2D);
				if (shadowTex2D == nullptr || mImpl->mShadowResolution2D != shadowTex2D->mWidth)
				{
					GPU::TextureDesc desc;

					mImpl->mShadowTex2D = builder.CreateTexture("ShadowMap2D", &desc);
				}



				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});
	}

	U32x2 GetInternalResolution()
	{
		return U32x2(
			(U32)(mImpl->mWindowSize.x() * mImpl->mResolutionScale),
			(U32)(mImpl->mWindowSize.y() * mImpl->mResolutionScale)
		);
	}

	void SetWindow(const GameWindow& gameWindow)
	{
		GPU::ResHandle swapChain = GPU::GetSwapChain();

		Platform::WindowRect clientRect = gameWindow.GetClientBounds();
		U32x2 clientSize = {
			(U32)(clientRect.mRight - clientRect.mLeft),
			(U32)(clientRect.mBottom - clientRect.mTop)
		};
		if (clientSize != mImpl->mWindowSize)
		{
			mImpl->mWindowSize = clientSize;
			GPU::ResizeSwapChain(swapChain, mImpl->mWindowSize.x(), mImpl->mWindowSize.y());
		}
	}
}
}