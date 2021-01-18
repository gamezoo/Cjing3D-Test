#include "renderGraph.h"
#include "core\memory\memory.h"
#include "core\memory\linearAllocator.h"
#include "core\container\dynamicArray.h"
#include "core\container\hashMap.h"
#include "core\container\set.h"
#include "core\jobsystem\jobsystem.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	////////////////////////////////////////////////////////////////////////////////////////////
	// Impl
	////////////////////////////////////////////////////////////////////////////////////////////
	struct RenderPassInst
	{
		String mName;
		RenderPass* mRenderPass = nullptr;
		I32 mIndex = 0;
	};

	struct ResourceInst
	{
		GPU::ResHandle mHandle = GPU::ResHandle::INVALID_HANDLE;
		I32 mIndex = -1;
		GPU::ResourceType mType;
		String mName;
		bool mIsUsed = false;

		GPU::TextureDesc mTexDesc;
		GPU::BufferDesc mBufferDesc;
	};
	
	class RenderGraphImpl
	{
	public:
		LinearAllocator mAllocator;
		DynamicArray<RenderPassInst> mRenderPasses;
		HashMap<String, I32> mRenderPassIndexMap;
		DynamicArray<RenderPassInst*> mExecuteRenderPasses;
		DynamicArray<GPU::CommandList*> mExecuteCmds;
		Set<I32> mNeededResources;

		// resource, build by setup
		DynamicArray<ResourceInst> mResources;
		DynamicArray<ResourceInst> mUsedResources;

		volatile I32 mCompileFailCount = 0;

		RenderGraphImpl() {
			mAllocator.Reserve(1024 * 1024);
		}

		void AddDependentRenderPass(DynamicArray<RenderPassInst*>& outRenderPasses, const Span<const RenderGraphResource>& resources);
		void FilterRenderPass(DynamicArray<RenderPassInst*>& outRenderPasses);
		bool CreateResource(ResourceInst& resInst);
		void RefreshResources();
	};

	void RenderGraphImpl::AddDependentRenderPass(DynamicArray<RenderPassInst*>& outRenderPasses, const Span<const RenderGraphResource>& resources)
	{
		// 如果renderPass的ouputs中包含了resources的资源，则将其添加到outRenderPasses中
		// 并以renderPass的inputs递归调用AddDependentRenderPass
		Set<I32> resourceHashSet;
		for (const auto& res : resources) 
		{
			mNeededResources.insert(res.mIndex);
			resourceHashSet.insert(res.Hash());
		}

		I32 startIndex = outRenderPasses.size();
		for (auto& renderPassInst : mRenderPasses)
		{
			auto outputs = renderPassInst.mRenderPass->GetOutputs();
			for (const RenderGraphResource& res : outputs)
			{
				if (resourceHashSet.find(res.Hash()) != nullptr) {
					outRenderPasses.push(&renderPassInst);
				}
			}
		}

		I32 endIndex = outRenderPasses.size();
		for (int i = startIndex; i < endIndex; i++) {
			AddDependentRenderPass(outRenderPasses, outRenderPasses[i]->mRenderPass->GetInputs());
		}
	}

	void RenderGraphImpl::FilterRenderPass(DynamicArray<RenderPassInst*>& outRenderPasses)
	{
		// filter render passes, remove same render passes
		Set<I32> renderPassSet;
		auto tempRenderPasses = outRenderPasses;
		outRenderPasses.end();

		for (const auto& renderPassInst : tempRenderPasses)
		{
			if (renderPassSet.find(renderPassInst->mIndex) != nullptr) {
				continue;
			}

			outRenderPasses.push(renderPassInst);
			renderPassSet.insert(renderPassInst->mIndex);
		}
	}

	bool RenderGraphImpl::CreateResource(ResourceInst& resInst)
	{
		switch (resInst.mType)
		{
		case GPU::RESOURCETYPE_BUFFER:
			resInst.mHandle = GPU::CreateBuffer(&resInst.mBufferDesc, nullptr, resInst.mName.c_str());
			break;
		case GPU::RESOURCETYPE_TEXTURE:
			resInst.mHandle = GPU::CreateTexture(&resInst.mTexDesc, nullptr, resInst.mName.c_str());
			break;
		default:
			break;
		}
		return resInst.mHandle != GPU::ResHandle::INVALID_HANDLE;
	}

	void RenderGraphImpl::RefreshResources()
	{
		for (const auto& resIndex : mNeededResources)
		{
			auto& resInst = mResources[resIndex];

			ResourceInst* usedResInstPtr = nullptr;
			for (auto& usedResInst : mUsedResources)
			{
				if (usedResInst.mIndex == resInst.mIndex &&
					usedResInst.mHandle != GPU::ResHandle::INVALID_HANDLE &&
					usedResInst.mIsUsed == false)
				{
					usedResInstPtr = &usedResInst;
					break;
				}
			}

			if (usedResInstPtr != nullptr)
			{
				usedResInstPtr->mIsUsed = true;
				resInst.mHandle = usedResInstPtr->mHandle;
			}
			else
			{
				if (resInst.mHandle != GPU::ResHandle::INVALID_HANDLE)
				{
					if (CreateResource(resInst)) 
					{
						resInst.mIsUsed = true;
						mUsedResources.push(resInst);
					}
				}
			}
		}

		// clear useless resources
		auto it = mUsedResources.begin();
		while (it != mUsedResources.end())
		{
			if (!it->mIsUsed)
			{
				if (it->mHandle != GPU::ResHandle::INVALID_HANDLE) {
					GPU::DestroyResource(it->mHandle);
				}
				it = mUsedResources.erase(it);
			}
			else {
				it++;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// RenderGraphResBuilder
	////////////////////////////////////////////////////////////////////////////////////////////
	RenderGraphResBuilder::RenderGraphResBuilder(RenderGraphImpl& renderGraph, RenderPass* renderPass) :
		mImpl(renderGraph),
		mRenderPass(renderPass)
	{
	}

	RenderGraphResBuilder::~RenderGraphResBuilder()
	{
	}

	RenderGraphResource RenderGraphResBuilder::CreateTexture(const char* name, const GPU::TextureDesc* desc)
	{
		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl.mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_TEXTURE;
		resInst.mTexDesc = *desc;
		mImpl.mResources.push(resInst);
		return RenderGraphResource(resInst.mIndex);
	}

	RenderGraphResource RenderGraphResBuilder::CreateBuffer(const char* name, const GPU::BufferDesc* desc)
	{
		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl.mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_BUFFER;
		resInst.mBufferDesc = *desc;
		mImpl.mResources.push(resInst);
		return RenderGraphResource(resInst.mIndex);
	}

	RenderGraphResource RenderGraphResBuilder::AddInput(RenderGraphResource res, GPU::BIND_FLAG bindFlag)
	{
		if (!res) {
			return res;
		}

		auto& resource = mImpl.mResources[res.mIndex];
		switch (resource.mType)
		{
		case GPU::RESOURCETYPE_BUFFER:
			resource.mBufferDesc.mBindFlags |= bindFlag;
			break;
		case GPU::RESOURCETYPE_TEXTURE:
			resource.mTexDesc.mBindFlags |= bindFlag;
			break;
		default:
			break;
		}

		mRenderPass->AddInput(res);
		return res;
	}

	RenderGraphResource RenderGraphResBuilder::AddOutput(RenderGraphResource res, GPU::BIND_FLAG bindFlag)
	{
		if (!res) {
			return res;
		}

		auto& resource = mImpl.mResources[res.mIndex];
		switch (resource.mType)
		{
		case GPU::RESOURCETYPE_BUFFER:
			resource.mBufferDesc.mBindFlags |= bindFlag;
			break;
		case GPU::RESOURCETYPE_TEXTURE:
			resource.mTexDesc.mBindFlags |= bindFlag;
			break;
		default:
			break;
		}

		// update version for trace renderPass resource dependencies
		mRenderPass->AddInput(res);
		res.mVersion++;
		mRenderPass->AddOutput(res);
		return res;
	}

	RenderGraphResource RenderGraphResBuilder::AddRTV(RenderGraphResource res, RenderGraphFrameAttachment attachment)
	{
		if (!res) {
			return res;
		}

		auto& resource = mImpl.mResources[res.mIndex];
		switch (resource.mType)
		{
		case GPU::RESOURCETYPE_TEXTURE:
			resource.mTexDesc.mBindFlags |= GPU::BIND_FLAG::BIND_RENDER_TARGET;
			break;
		default:
			Debug::Warning("Invalid res \'%d\', nnly texture could bind render target", (I32)resource.mType);
			break;
		}

		// renderPass add rtv
		mRenderPass->AddRTV(res, attachment);

		// update version for trace renderPass resource dependencies
		mRenderPass->AddInput(res);
		res.mVersion++;
		mRenderPass->AddOutput(res);
		return res;
	}

	RenderGraphResource RenderGraphResBuilder::SetDSV(RenderGraphResource res, RenderGraphFrameAttachment attachment)
	{
		if (!res) {
			return res;
		}

		auto& resource = mImpl.mResources[res.mIndex];
		switch (resource.mType)
		{
		case GPU::RESOURCETYPE_TEXTURE:
			resource.mTexDesc.mBindFlags |= GPU::BIND_FLAG::BIND_DEPTH_STENCIL;
			break;
		default:
			Debug::Warning("Invalid res \'%d\', nnly texture could bind depth stencil", (I32)resource.mType);
			break;
		}

		// renderPass add dsv
		mRenderPass->SetDSV(res, attachment);

		// update for trace renderPass resource dependencies
		mRenderPass->AddInput(res);
		res.mVersion++;
		mRenderPass->AddOutput(res);

		return res;
	}

	const GPU::TextureDesc* RenderGraphResBuilder::GetTextureDesc(RenderGraphResource res)const
	{
		if (!res) {
			return nullptr;
		}

		if (res.mIndex >= mImpl.mResources.size()) {
			return nullptr;
		}

		auto& resInst = mImpl.mResources[res.mIndex];
		if (resInst.mType != GPU::RESOURCETYPE_TEXTURE) {
			return nullptr;
		}

		return &resInst.mTexDesc;
	}

	const GPU::BufferDesc* RenderGraphResBuilder::GetBufferDesc(RenderGraphResource res)const
	{
		if (!res) {
			return nullptr;
		}

		if (res.mIndex >= mImpl.mResources.size()) {
			return nullptr;
		}
	
		auto& resInst = mImpl.mResources[res.mIndex];
		if (resInst.mType != GPU::RESOURCETYPE_BUFFER) {
			return nullptr;
		}

		return &resInst.mBufferDesc;
	}

	void* RenderGraphResBuilder::Alloc(I32 size)
	{
		return mImpl.mAllocator.Allocate(size);
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// RenderGraph
	////////////////////////////////////////////////////////////////////////////////////////////

	RenderGraph::RenderGraph()
	{
		mImpl = CJING_NEW(RenderGraphImpl);
	}

	RenderGraph::~RenderGraph()
	{
		Clear();
		for (auto& resInst : mImpl->mUsedResources) {
			if (resInst.mHandle != GPU::ResHandle::INVALID_HANDLE) {
				GPU::DestroyResource(resInst.mHandle);
			}
		}
		CJING_SAFE_DELETE(mImpl);
	}

	RenderGraphResource RenderGraph::ImportTexture(const char* name, GPU::ResHandle handle, const GPU::TextureDesc* desc)
	{
		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl->mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_TEXTURE;
		resInst.mTexDesc = *desc;
		resInst.mHandle = handle;
		mImpl->mResources.push(resInst);
		return RenderGraphResource(resInst.mIndex);
	}

	RenderGraphResource RenderGraph::ImportBuffer(const char* name, GPU::ResHandle handle, const GPU::BufferDesc* desc)
	{
		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl->mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_BUFFER;
		resInst.mBufferDesc = *desc;
		resInst.mHandle = handle;
		mImpl->mResources.push(resInst);
		return RenderGraphResource(resInst.mIndex);
	}

	void RenderGraph::AddRenderPass(const char* name, RenderPass* renderPass)
	{
		auto it = mImpl->mRenderPassIndexMap.find(name);
		if (it != nullptr)
		{
			Debug::Warning("The render pass \'%s\' is already exists.", name);
			return;
		}

		I32 index = mImpl->mRenderPasses.size();
		auto& passInst = mImpl->mRenderPasses.emplace();
		passInst.mName = name;
		passInst.mIndex = index;
		passInst.mRenderPass = renderPass;

		mImpl->mRenderPassIndexMap.insert(name, index);
	}

	bool RenderGraph::Execute(RenderGraphResource finalRes)
	{
		mImpl->mExecuteRenderPasses.clear();
		mImpl->mExecuteCmds.clear();
		mImpl->mNeededResources.clear();

		// 先找到各个renderPass所有的newest finalRes 
		finalRes.mVersion = -1;
		for (const auto& renderPass : mImpl->mRenderPasses)
		{
			auto outputs = renderPass.mRenderPass->GetOutputs();
			for (const RenderGraphResource& res : outputs)
			{
				if (res.mIndex == finalRes.mIndex && res.mVersion > finalRes.mVersion) {
					finalRes = res;
				}
			}
		}

		if (finalRes.mVersion < 0) 
		{
			Debug::Error("Invalid final resource for render graph to execute");
			return false;
		}

		// 根据finalRes和依赖关系，筛选出所有有效的renderPass
		mImpl->mExecuteRenderPasses.reserve(mImpl->mRenderPasses.size());
		mImpl->AddDependentRenderPass(mImpl->mExecuteRenderPasses, Span(&finalRes, 1));
		mImpl->FilterRenderPass(mImpl->mExecuteRenderPasses);

		// 创建所有renderPasses所需的resources
		mImpl->RefreshResources();

		// execute all renderPasses by jobsystem
		I32 renderPassCount = mImpl->mExecuteRenderPasses.size();
		if (mImpl->mExecuteCmds.size() < renderPassCount)
		{
			mImpl->mExecuteCmds.resize(renderPassCount);
			for (int i = 0; i < renderPassCount; i++) {
				mImpl->mExecuteCmds[i] = GPU::CreateCommandlist();
			}
		}

		JobSystem::JobHandle jobHandle;
		DynamicArray<JobSystem::JobInfo> passJobs;
		passJobs.reserve(renderPassCount);
		for (int i = 0; i < renderPassCount; i++)
		{
			auto& jobInfo = passJobs.emplace();
			jobInfo.jobName = mImpl->mExecuteRenderPasses[i]->mName;
			jobInfo.userParam_ = i;
			jobInfo.userData_ = mImpl;
			jobInfo.jobFunc_ = [](I32 param, void* data)
			{
				RenderGraphImpl* impl = reinterpret_cast<RenderGraphImpl*>(data);
				if (!impl) {
					return;
				}

				RenderPassInst* renderPassInst = impl->mExecuteRenderPasses[param];
				GPU::CommandList* cmd = impl->mExecuteCmds[param];
				{
					auto ent = cmd->Event(renderPassInst->mName);
					renderPassInst->mRenderPass->Execute(*cmd);
				}

				if (!cmd->GetCommands().empty())
				{
					if (!GPU::CompileCommandList(*cmd))
					{
						Concurrency::AtomicIncrement(&impl->mCompileFailCount);
						Debug::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName);
					}
				}
			};
		}

		JobSystem::RunJobs(passJobs.data(), passJobs.size(), &jobHandle);
		JobSystem::Wait(&jobHandle);

		if (mImpl->mCompileFailCount > 0) {
			return false;
		}

		// submit all cmd of render pass
		DynamicArray<GPU::CommandList*> cmdsToSubmit;
		for (int i = 0; i < renderPassCount; i++)
		{
			GPU::CommandList* cmd = mImpl->mExecuteCmds[i];
			if (!cmd->GetCommands().empty()) {
				cmdsToSubmit.push(cmd);
			}
		}
		if (!GPU::SubmitCommandList(Span(cmdsToSubmit.data(), cmdsToSubmit.size())))
		{
			Debug::Warning("Failed to submit command lists.");
			return false;
		}

		return true;
	}

	void RenderGraph::Clear()
	{
		for (auto& renderPassInst : mImpl->mRenderPasses) {
			renderPassInst.mRenderPass->~RenderPass();
		}

		// renderPass
		mImpl->mRenderPasses.clear();
		mImpl->mExecuteRenderPasses.clear();
		mImpl->mRenderPassIndexMap.clear();

		// res
		for (auto& resInst : mImpl->mUsedResources) {
			resInst.mIsUsed = false;
		}
		mImpl->mNeededResources.clear();
		mImpl->mResources.clear();

		mImpl->mAllocator.Reset();
	}

	void* RenderGraph::Allocate(size_t size)
	{
		return (void*)mImpl->mAllocator.Allocate(size);
	}
}