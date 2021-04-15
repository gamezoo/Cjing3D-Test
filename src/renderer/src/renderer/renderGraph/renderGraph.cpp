#include "renderGraph.h"
#include "renderPassImpl.h"
#include "denpendencyGraph.h"
#include "core\memory\memory.h"
#include "core\memory\linearAllocator.h"
#include "core\container\dynamicArray.h"
#include "core\container\hashMap.h"
#include "core\container\set.h"
#include "core\concurrency\jobsystem.h"
#include "core\helper\profiler.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	namespace
	{
		bool operator==(const GPU::TextureDesc& a, const GPU::TextureDesc& b)
		{
			return memcmp(&a, &b, sizeof(a)) == 0;
		}

		bool operator==(const GPU::BufferDesc& a, const GPU::BufferDesc& b)
		{
			return memcmp(&a, &b, sizeof(a)) == 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Impl
	////////////////////////////////////////////////////////////////////////////////////////////
	struct RenderPassInst
	{
		StaticString<64> mName;
		RenderPass* mRenderPass = nullptr;
		I32 mIndex = 0;
	};

	struct ResourceInst
	{
		GPU::ResHandle mHandle = GPU::ResHandle::INVALID_HANDLE;
		I32 mIndex = -1;
		GPU::ResourceType mType;

		StaticString<64> mName;
		I32 mVersion = 0;
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

		// resource, build by setup
		Set<I32> mNeededResources;
		DynamicArray<ResourceInst> mResources;
		DynamicArray<ResourceInst> mTransientResources;

		DenpendencyGraph mDenpendencyGraph;

		volatile I32 mCompileFailCount = 0;

		RenderGraphImpl() {
			mAllocator.Reserve(1024 * 1024);
		}

		void AddDependentRenderPass(DynamicArray<RenderPassInst*>& outRenderPasses, const Span<const RenderGraphResource>& resources);
		void FilterRenderPass(DynamicArray<RenderPassInst*>& outRenderPasses);
		bool CreateResource(ResourceInst& resInst);
		void RefreshResources();
		void CreateFrameBindingSets();

		GPU::ResHandle GetTexture(const RenderGraphResource& res, GPU::TextureDesc* outDesc = nullptr)
		{
			const auto& resInst = mResources[res.mIndex];
			Debug::CheckAssertion(resInst.mType == GPU::RESOURCETYPE_TEXTURE);
			if (outDesc != nullptr) {
				*outDesc = resInst.mTexDesc;
			}
			return resInst.mHandle;
		}

		GPU::ResHandle GetBuffer(const RenderGraphResource& res, GPU::BufferDesc* outDesc = nullptr)
		{
			const auto& resInst = mResources[res.mIndex];
			Debug::CheckAssertion(resInst.mType == GPU::RESOURCETYPE_BUFFER);
			if (outDesc != nullptr) {
				*outDesc = resInst.mBufferDesc;
			}
			return resInst.mHandle;
		}
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
		outRenderPasses.clear();

		// reverse render passes
		for (int i = tempRenderPasses.size() - 1; i >= 0; i--)
		{
			auto& renderPassInst = tempRenderPasses[i];
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
			for (auto& usedResInst : mTransientResources)
			{
				if (usedResInst.mHandle != GPU::ResHandle::INVALID_HANDLE &&
					usedResInst.mType == resInst.mType &&
					usedResInst.mIsUsed == false)
				{
					bool find = false;
					switch (resInst.mType)
					{
					case GPU::RESOURCETYPE_BUFFER:
						find = resInst.mBufferDesc == usedResInst.mBufferDesc;
						break;
					case GPU::RESOURCETYPE_TEXTURE:
						find = resInst.mTexDesc == usedResInst.mTexDesc;
						break;
					default:
						break;
					}
					if (find)
					{
						usedResInstPtr = &usedResInst;
						break;
					}
				}
			}

			if (usedResInstPtr != nullptr)
			{
				usedResInstPtr->mIsUsed = true;
				resInst.mHandle = usedResInstPtr->mHandle;
			}
			else
			{
				if (resInst.mHandle == GPU::ResHandle::INVALID_HANDLE)
				{
					if (CreateResource(resInst)) 
					{
						resInst.mIsUsed = true;
						mTransientResources.push(resInst);
					}
				}
			}
		}

		// clear useless resources
		auto it = mTransientResources.begin();
		while (it != mTransientResources.end())
		{
			if (!it->mIsUsed)
			{
				if (it->mHandle != GPU::ResHandle::INVALID_HANDLE) 
				{
					GPU::DestroyResource(it->mHandle);
					it->mHandle = GPU::ResHandle::INVALID_HANDLE;
				}
				it = mTransientResources.erase(it);
			}
			else {
				it++;
			}
		}
	}

	void RenderGraphImpl::CreateFrameBindingSets()
	{
		for (auto& rendrPassInst : mRenderPasses)
		{
			auto* renderPass = rendrPassInst.mRenderPass->mImpl;
			auto& frameBindingSetDesc = renderPass->mFrameBindingSetDesc;
			frameBindingSetDesc.mAttachments.reserve(renderPass->mRTVCount + 1);

			// rtvs
			for (int i = 0; i < renderPass->mRTVCount; i++)
			{
				GPU::BindingFrameAttachment attachment;
				RenderGraphResource rtvRes = renderPass->mRTVs[i];
				if (rtvRes) {
					attachment.mResource = GetTexture(rtvRes);
				}

				auto& graphAttachment = renderPass->mRTVAttachments[i];
				attachment.mLoadOperator = graphAttachment.mLoadOperator;
				attachment.mType = GPU::BindingFrameAttachment::RENDERTARGET;
				frameBindingSetDesc.mAttachments.push(attachment);
			}

			// dsv
			if (renderPass->mDSV)
			{
				GPU::BindingFrameAttachment attachment;
				attachment.mLoadOperator = renderPass->mDSVAttachment.mLoadOperator;
				attachment.mType = GPU::BindingFrameAttachment::DEPTH_STENCIL;
				attachment.mResource = GetTexture(renderPass->mDSV);
				frameBindingSetDesc.mAttachments.push(attachment);
			}

			if (!frameBindingSetDesc.mAttachments.empty()) {
				renderPass->mFrameBindingSet = GPU::CreateFrameBindingSet(&frameBindingSetDesc);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// RenderResources
	////////////////////////////////////////////////////////////////////////////////////////////
	RenderGraphResources::RenderGraphResources(RenderGraphImpl& renderGraph, RenderPass* renderPass) :
		mImpl(renderGraph),
		mRenderPass(renderPass)
	{
	}

	RenderGraphResources::~RenderGraphResources()
	{
	}

	GPU::ResHandle RenderGraphResources::GetFrameBindingSet() const
	{
		return mRenderPass->mImpl->mFrameBindingSet;
	}

	GPU::ResHandle RenderGraphResources::GetBuffer(RenderGraphResource res, GPU::BufferDesc* outDesc)
	{
		return mImpl.GetBuffer(res, outDesc);
	}

	GPU::ResHandle RenderGraphResources::GetTexture(RenderGraphResource res, GPU::TextureDesc* outDesc)
	{
		return mImpl.GetTexture(res, outDesc);
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

		if (bindFlag != GPU::BIND_NOTHING)
		{
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
		}

		mRenderPass->AddInput(res);
		return res;
	}

	RenderGraphResource RenderGraphResBuilder::AddOutput(RenderGraphResource res, GPU::BIND_FLAG bindFlag)
	{
		if (!res) {
			return res;
		}

		if (bindFlag != GPU::BIND_NOTHING)
		{
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
			Logger::Warning("Invalid res \'%d\', nnly texture could bind render target", (I32)resource.mType);
			break;
		}

		// renderPass add rtv
		mRenderPass->AddRTV(res, attachment);

		// update version for trace renderPass resource dependencies
		mRenderPass->AddInput(res);
		res.mVersion++;
		mRenderPass->AddOutput(res);

		// update graphs res version (TODO?)
		resource.mVersion = res.mVersion;

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
			Logger::Warning("Invalid res \'%d\', nnly texture could bind depth stencil", (I32)resource.mType);
			break;
		}

		// renderPass add dsv
		mRenderPass->SetDSV(res, attachment);

		// update for trace renderPass resource dependencies
		mRenderPass->AddInput(res);
		res.mVersion++;
		mRenderPass->AddOutput(res);

		// update graphs res version (TODO?)
		resource.mVersion = res.mVersion;

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

		for (auto& resInst : mImpl->mTransientResources) {
			if (resInst.mHandle != GPU::ResHandle::INVALID_HANDLE) 
			{
				GPU::DestroyResource(resInst.mHandle);
				resInst.mHandle = GPU::ResHandle::INVALID_HANDLE;
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
		resInst.mTexDesc = desc != nullptr ? *desc : *GPU::GetTextureDesc(handle);
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
		resInst.mBufferDesc = desc != nullptr ? *desc : *GPU::GetBufferDesc(handle);
		resInst.mHandle = handle;
		mImpl->mResources.push(resInst);
		return RenderGraphResource(resInst.mIndex);
	}

	void RenderGraph::AddRenderPass(const char* name, RenderPass* renderPass)
	{
		auto it = mImpl->mRenderPassIndexMap.find(name);
		if (it != nullptr)
		{
#ifndef DEBUG
			Logger::Warning("The render pass \'%s\' is already exists.", name);
#endif
			return;
		}

		I32 index = mImpl->mRenderPasses.size();
		auto& passInst = mImpl->mRenderPasses.emplace();
		passInst.mName = name;
		passInst.mIndex = index;
		passInst.mRenderPass = renderPass;

		mImpl->mRenderPassIndexMap.insert(name, index);
	}

	void RenderGraph::Present(RenderGraphResource res)
	{
		AddPresentRenderPass(nullptr, [res](RenderGraphResBuilder& builder) {
			builder.AddInput(res);
		});
	}

	bool RenderGraph::Compile()
	{
		PROFILE_CPU_BLOCK("RenderGraphCompile");

		mImpl->mExecuteRenderPasses.clear();
		mImpl->mExecuteCmds.clear();
		mImpl->mNeededResources.clear();

		
	
		

		return true;
	}

	bool RenderGraph::Execute()
	{
		PROFILE_CPU_BLOCK("RenderGraphExecute");

		// execute all renderPasses by jobsystem
		I32 renderPassCount = mImpl->mExecuteRenderPasses.size();
		if (mImpl->mExecuteCmds.size() < renderPassCount)
		{
			mImpl->mExecuteCmds.resize(renderPassCount);
			for (int i = 0; i < renderPassCount; i++) {
				mImpl->mExecuteCmds[i] = GPU::CreateCommandlist();
			}
		}

		JobSystem::JobHandle jobHandle = JobSystem::INVALID_HANDLE;
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

				// execute render pass
				RenderPassInst* renderPassInst = impl->mExecuteRenderPasses[param];
				GPU::CommandList* cmd = impl->mExecuteCmds[param];
				{
					auto ent = cmd->Event(renderPassInst->mName);
					RenderGraphResources resources(*impl, renderPassInst->mRenderPass);
					renderPassInst->mRenderPass->Execute(resources, *cmd);
				}

				// compile cmd
				if (!cmd->GetCommands().empty())
				{
					if (!GPU::CompileCommandList(*cmd))
					{
						Concurrency::AtomicIncrement(&impl->mCompileFailCount);
						Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
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
			Logger::Warning("Failed to submit command lists.");
			return false;
		}

		return true;
	}

	bool RenderGraph::Execute(RenderGraphResource finalRes)
	{
		{
			PROFILE_CPU_BLOCK("RenderGraphCompile");

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
				Logger::Warning("Invalid final resource for render graph to execute");
				return false;
			}

			// 根据finalRes和依赖关系，筛选出所有有效的renderPass
			mImpl->mExecuteRenderPasses.reserve(mImpl->mRenderPasses.size());
			mImpl->AddDependentRenderPass(mImpl->mExecuteRenderPasses, Span(&finalRes, 1));
			mImpl->FilterRenderPass(mImpl->mExecuteRenderPasses);

			// 创建所有renderPasses所需的resources
			mImpl->RefreshResources();

			// create frameBindingSet for each renderPass
			mImpl->CreateFrameBindingSets();
		}

		{
			PROFILE_CPU_BLOCK("RenderGraphExecute");

			// execute all renderPasses by jobsystem
			I32 renderPassCount = mImpl->mExecuteRenderPasses.size();
			if (mImpl->mExecuteCmds.size() < renderPassCount)
			{
				mImpl->mExecuteCmds.resize(renderPassCount);
				for (int i = 0; i < renderPassCount; i++) {
					mImpl->mExecuteCmds[i] = GPU::CreateCommandlist();
				}
			}

			JobSystem::JobHandle jobHandle = JobSystem::INVALID_HANDLE;
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

					// execute render pass
					RenderPassInst* renderPassInst = impl->mExecuteRenderPasses[param];
					GPU::CommandList* cmd = impl->mExecuteCmds[param];
					{
						auto ent = cmd->Event(renderPassInst->mName);
						RenderGraphResources resources(*impl, renderPassInst->mRenderPass);
						renderPassInst->mRenderPass->Execute(resources, *cmd);
					}

					// compile cmd
					if (!cmd->GetCommands().empty())
					{
						if (!GPU::CompileCommandList(*cmd))
						{
							Concurrency::AtomicIncrement(&impl->mCompileFailCount);
							Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
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
				Logger::Warning("Failed to submit command lists.");
				return false;
			}
		}

		return true;
	}

	bool RenderGraph::Execute(Span<RenderGraphResource> finalResources)
	{
		{
			PROFILE_CPU_BLOCK("RenderGraphCompile");

			mImpl->mExecuteRenderPasses.clear();
			mImpl->mExecuteCmds.clear();
			mImpl->mNeededResources.clear();

			// 先找到各个renderPass所有的newest finalRes 
			for (RenderGraphResource& res : finalResources) {
				res.mVersion = -1;
			}

			for (const auto& renderPass : mImpl->mRenderPasses)
			{
				auto outputs = renderPass.mRenderPass->GetOutputs();
				for (const RenderGraphResource& res : outputs)
				{
					for (RenderGraphResource& finalRes : finalResources)
					{
						if (res.mIndex == finalRes.mIndex && res.mVersion > finalRes.mVersion) {
							finalRes = res;
						}
					}
				}
			}

			for (RenderGraphResource& finalRes : finalResources)
			{
				if (finalRes.mVersion < 0)
				{
					Logger::Warning("Invalid final resource for render graph to execute");
					return false;
				}
			}

			// 根据finalRes和依赖关系，筛选出所有有效的renderPass
			mImpl->mExecuteRenderPasses.reserve(mImpl->mRenderPasses.size());
			mImpl->AddDependentRenderPass(mImpl->mExecuteRenderPasses, finalResources);
			mImpl->FilterRenderPass(mImpl->mExecuteRenderPasses);

			// 创建所有renderPasses所需的resources
			mImpl->RefreshResources();

			// create frameBindingSet for each renderPass
			mImpl->CreateFrameBindingSets();
		}

		{
			PROFILE_CPU_BLOCK("RenderGraphExecute");

			// execute all renderPasses by jobsystem
			I32 renderPassCount = mImpl->mExecuteRenderPasses.size();
			if (mImpl->mExecuteCmds.size() < renderPassCount)
			{
				mImpl->mExecuteCmds.resize(renderPassCount);
				for (int i = 0; i < renderPassCount; i++) {
					mImpl->mExecuteCmds[i] = GPU::CreateCommandlist();
				}
			}

			JobSystem::JobHandle jobHandle = JobSystem::INVALID_HANDLE;
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

					// execute render pass
					RenderPassInst* renderPassInst = impl->mExecuteRenderPasses[param];
					GPU::CommandList* cmd = impl->mExecuteCmds[param];
					{
						auto ent = cmd->Event(renderPassInst->mName);
						RenderGraphResources resources(*impl, renderPassInst->mRenderPass);
						renderPassInst->mRenderPass->Execute(resources, *cmd);
					}

					// compile cmds
					if (!cmd->GetCommands().empty())
					{
						if (!GPU::CompileCommandList(*cmd))
						{
							Concurrency::AtomicIncrement(&impl->mCompileFailCount);
							Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
						}
					}
				};
			}

			JobSystem::RunJobs(passJobs.data(), passJobs.size(), &jobHandle);
			JobSystem::Wait(&jobHandle);

			if (mImpl->mCompileFailCount > 0) {
				return false;
			}

			// //当前阶段先不提交cmds，此时cmds缓存在GPUManager中，后续提交或者Present时，会全部提交
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
		for (auto& resInst : mImpl->mTransientResources) 
		{
			resInst.mIsUsed = false;
			resInst.mVersion = 0;
		}
		mImpl->mNeededResources.clear();
		mImpl->mResources.clear();

		mImpl->mAllocator.Reset();
	}

	void* RenderGraph::Allocate(size_t size)
	{
		return (void*)mImpl->mAllocator.Allocate(size);
	}

	RenderGraphResource RenderGraph::GetResource(const char* name)const
	{
		for (const auto& res : mImpl->mResources)
		{
			if (res.mName == name) {
				return RenderGraphResource(res.mIndex, res.mVersion);
			}
		}
		return RenderGraphResource();
	}
}