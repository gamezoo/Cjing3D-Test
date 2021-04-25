#include "renderGraph.h"
#include "renderPassImpl.h"
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
		enum { Unused = ~0u };

		StaticString<64> mName;
		RenderPass* mRenderPass = nullptr;
		I32 mIndex = 0;
		U32 mPhysicalIndex = Unused;
	};

	struct PhysicalRenderPass
	{
		DynamicArray<I32> mPasses;
	};

	struct ResourceInst
	{
		enum { Unused = ~0u };

		GPU::ResHandle mHandle = GPU::ResHandle::INVALID_HANDLE;
		I32 mIndex = -1;
		GPU::ResourceType mType;

		StaticString<64> mName;
		I32 mVersion = 0;
		bool mIsUsed = false;

		GPU::TextureDesc mTexDesc;
		GPU::BufferDesc mBufferDesc;

		// new
		U32 mPhysicalIndex = Unused;
		DynamicArray<I32> mWrittenPasses;
		DynamicArray<I32> mReadPasses;
	};

	RenderGraphResourceDimension CreateResDimension(const ResourceInst& res)
	{
		return RenderGraphResourceDimension();
	}

	struct RenderPassExecuteState
	{
		GPU::CommandList* mCmd = nullptr;
		bool mActive = false;

		void emitPreBarriers()
		{
		}

		void emitPostBarriers()
		{
		}
	};

	class RenderGraphImpl
	{
	public:
		LinearAllocator mAllocator;
		DynamicArray<RenderPassInst> mRenderPasses;
		HashMap<String, I32> mRenderPassIndexMap;
		DynamicArray<GPU::CommandList*> mExecuteCmds;
		DynamicArray<RenderPassInst*> mExecuteRenderPasses;

		// resource, build by setup
		Set<I32> mNeededResources;
		DynamicArray<ResourceInst> mResources;
		DynamicArray<ResourceInst> mTransientResources;

		volatile I32 mCompileFailCount = 0;

		// Refactor begin
		RenderGraphResource mFinalRes;
		HashMap<RenderGraphResource, I32> mResourceIndexMap;
		DynamicArray<I32> mPassStack;					// used for filter passes by finalRes
		DynamicArray<Set<I32>> mPassDependencies;
		DynamicArray<Set<I32>> mPassMergeDependencies;
		DynamicArray<RenderGraphResourceDimension> mPhysicalResourceDimensions;
		DynamicArray<PhysicalRenderPass> mPhysicalRenderPasses;
		DynamicArray<RenderPassExecuteState> mRenderPassExecuteStates;

	public:
		RenderGraphImpl() {
			mAllocator.Reserve(1024 * 1024);
		}

		bool Compile();
		bool Execute();
		void FilterRenderPass(DynamicArray<I32>& renderPasses);
		void ReorderRenderPass(DynamicArray<I32>& renderPasses);
		void BuildPhysicalResources();
		void BuildPhysicalPasses();
		void BuildTransients();
		void BuildRenderPassInfos();
		void BuildBarriers();
		void BuildAliases();

		void TraverseRenderPassDependency(const RenderPassInst& renderPass, U32 stackCount);
		void DependRenderPassRecursive(const RenderPassInst& renderPass, const DynamicArray<I32>& writtenPasses, U32 stackCount, bool mergeDependency);
		bool CheckPassDependent(I32 srcPass, I32 dstPass)const;

		bool CreateResource(ResourceInst& resInst);
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
				attachment.mLoadOp = graphAttachment.mLoadOp;
				attachment.mType = GPU::BindingFrameAttachment::RENDERTARGET;
				frameBindingSetDesc.mAttachments.push(attachment);
			}

			// dsv
			if (renderPass->mDSV)
			{
				GPU::BindingFrameAttachment attachment;
				attachment.mLoadOp = renderPass->mDSVAttachment.mLoadOp;
				attachment.mType = GPU::BindingFrameAttachment::DEPTH_STENCIL;
				attachment.mResource = GetTexture(renderPass->mDSV);
				frameBindingSetDesc.mAttachments.push(attachment);
			}

			if (!frameBindingSetDesc.mAttachments.empty()) {
				renderPass->mFrameBindingSet = GPU::CreateFrameBindingSet(&frameBindingSetDesc);
			}
		}
	}

	void RenderGraphImpl::TraverseRenderPassDependency(const RenderPassInst& renderPass, U32 stackCount)
	{
		// TODO: in dev
		auto inputs = renderPass.mRenderPass->GetInputs();
		for (const RenderGraphResource& res : inputs)
		{
			auto resIndex = mResourceIndexMap.find(res);
			if (resIndex != nullptr)
			{
				ResourceInst& res = mResources[*resIndex];
				DependRenderPassRecursive(renderPass, res.mWrittenPasses, stackCount, false);
			}
		}
	}

	void RenderGraphImpl::DependRenderPassRecursive(const RenderPassInst& renderPass, const DynamicArray<I32>& writtenPasses, U32 stackCount, bool mergeDependency)
	{
		for (auto& passIndex : writtenPasses)
		{
			if (passIndex != renderPass.mIndex) {
				mPassDependencies[renderPass.mIndex].insert(passIndex);
			}
		}

		if (mergeDependency)
		{
			for (auto& passIndex : writtenPasses)
			{
				if (passIndex != renderPass.mIndex) {
					mPassMergeDependencies[renderPass.mIndex].insert(passIndex);
				}
			}
		}

		stackCount++;

		for (auto& passIndex : writtenPasses)
		{
			mPassStack.push(passIndex);
			TraverseRenderPassDependency(mRenderPasses[passIndex], stackCount);
		}
	}

	void RenderGraphImpl::FilterRenderPass(DynamicArray<I32>& renderPasses)
	{
		// first reverse render passes
		std::reverse(renderPasses.begin(), renderPasses.end());

		// filter same render passes
		Set<I32> renderPassSet;
		I32* availableIt = renderPasses.begin();
		for (const auto& pass : renderPasses)
		{
			if (renderPassSet.find(pass) == nullptr)
			{
				renderPassSet.insert(pass);
				*availableIt = pass;
				availableIt++;
			}
		}
		renderPasses.erase(availableIt, renderPasses.end());
	}

	void RenderGraphImpl::ReorderRenderPass(DynamicArray<I32>& renderPasses)
	{
		PROFILE_FUNCTION()

		for (I32 passIndex = 0; passIndex < mPassMergeDependencies.size(); passIndex++)
		{
			auto& passDeps = mPassDependencies[passIndex];
			for (auto mergeDep : mPassMergeDependencies[passIndex])
			{
				for (auto dep : passDeps)
				{
					if (mergeDep == dep) {
						continue;
					}

					if (CheckPassDependent(mergeDep, dep)) {
						continue;
					}

					mPassDependencies[mergeDep].insert(dep);
				}
			}
		}

		DynamicArray<I32> unscheduledPasses;
		unscheduledPasses.reserve(renderPasses.size());
		std::swap(unscheduledPasses, renderPasses);

		auto SchedulePass = [&](I32 index) {

			renderPasses.push(unscheduledPasses[index]);
			unscheduledPasses.eraseItem(index);
		};
		SchedulePass(0);

		while (!unscheduledPasses.empty())
		{
			I32 bestFactor = 0;
			I32 bestIndex = 0;
			for (I32 index = 0; index < unscheduledPasses.size(); index++)
			{
				I32 unschedulePassIndex = unscheduledPasses[index];
				I32 currentFactor = 0;

				if (mPassMergeDependencies[unschedulePassIndex].find(renderPasses.back()))
				{
					bestFactor = ~0u;
				}
				else
				{
					for (int j = renderPasses.size() - 1; j >= 0; j--)
					{
						if (CheckPassDependent(unscheduledPasses[j], unscheduledPasses[index])) {
							break;
						}
						currentFactor++;
					}
				}

				if (currentFactor <= bestFactor) {
					continue;
				}

				// TODO: very inEfficient
				bool available = true;
				for (int j = 0; j < index; j++)
				{
					if (CheckPassDependent(unscheduledPasses[j], unscheduledPasses[index]))
					{
						available = false;
						break;
					}
				}
				if (!available) {
					continue;
				}

				bestIndex = index;
				bestFactor = currentFactor;
			}

			SchedulePass(bestIndex);
		}
	}

	void RenderGraphImpl::BuildPhysicalResources()
	{
		for (auto& passIndex : mPassStack)
		{
			// inputs
			RenderPassInst& renderPass = mRenderPasses[passIndex];
			auto inputs = renderPass.mRenderPass->GetInputs();
			for (auto& input : inputs)
			{
				auto resIndex = mResourceIndexMap.find(input);
				if (resIndex != nullptr)
				{
					auto& res = mResources[*resIndex];
					if (res.mPhysicalIndex == ResourceInst::Unused)
					{
						RenderGraphResourceDimension dim = CreateResDimension(res);
						mPhysicalResourceDimensions.push(dim);
					}
					res.mPhysicalIndex++;
				}
			}

			// outputs
			auto outputs = renderPass.mRenderPass->GetOutputs();
			for (auto& output : outputs)
			{
				auto resIndex = mResourceIndexMap.find(output);
				if (resIndex != nullptr)
				{
					auto& res = mResources[*resIndex];
					if (res.mPhysicalIndex == ResourceInst::Unused)
					{
						RenderGraphResourceDimension dim = CreateResDimension(res);
						mPhysicalResourceDimensions.push(dim);
					}
					res.mPhysicalIndex++;
				}
			}

			// rtvs

			// dsv
		}
	}

	void RenderGraphImpl::BuildPhysicalPasses()
	{
		mPhysicalRenderPasses.clear();

		auto CheckMerge = [&](const RenderPassInst& prev, const RenderPassInst& next) {
			return false;
		};
		for(I32 index = 0; index < mPassStack.size();)
		{
			PhysicalRenderPass physicalRenderPass;
			I32 mergeEnd = index + 1;
			for(; mergeEnd < mPassStack.size(); mergeEnd++)
			{
				physicalRenderPass.mPasses.push(mPassStack[mergeEnd - 1]);

				bool mergeEnable = true;
				for(I32 mergeIndex = index; mergeIndex < mergeEnd; mergeIndex++)
				{
					if (!CheckMerge(mRenderPasses[mPassStack[mergeIndex]], mRenderPasses[mPassStack[mergeEnd]]))
					{
						mergeEnable = false;
						break;
					}
				}
				if (!mergeEnable) {
					break;
				}
			}

			mPhysicalRenderPasses.push(physicalRenderPass);
			index = mergeEnd;
		}

		for (U32 i = 0; i < mPhysicalRenderPasses.size(); i++)
		{
			for (I32 passIndex : mPhysicalRenderPasses[i].mPasses) {
				mRenderPasses[passIndex].mPhysicalIndex = i;
			}
		}
	}

	void RenderGraphImpl::BuildTransients()
	{
		DynamicArray<U32> bePhysicalPassUsed;
		bePhysicalPassUsed.resize(mPhysicalResourceDimensions.size());
		for (auto& v : bePhysicalPassUsed) {
			v = RenderPassInst::Unused;
		}

		for (auto& dim : mPhysicalResourceDimensions)
		{
			dim.mIsTransient = true;
		}

		for (auto& res : mResources)
		{
			if (res.mType != GPU::RESOURCETYPE_TEXTURE) {
				continue;
			}

			if (res.mPhysicalIndex == ResourceInst::Unused) {
				continue;
			}

			for (auto& passIndex : res.mWrittenPasses)
			{
				U32 passPhysicalIndex = mRenderPasses[passIndex].mPhysicalIndex;
				if (passPhysicalIndex != RenderPassInst::Unused)
				{
					if (bePhysicalPassUsed[res.mPhysicalIndex] != RenderPassInst::Unused &&
						passPhysicalIndex != bePhysicalPassUsed[res.mPhysicalIndex])
					{
						mPhysicalResourceDimensions[res.mPhysicalIndex].mIsTransient = false;
						break;
					}
					bePhysicalPassUsed[res.mPhysicalIndex] = passPhysicalIndex;
				}
			}

			for (auto& passIndex : res.mReadPasses)
			{
				U32 passPhysicalIndex = mRenderPasses[passIndex].mPhysicalIndex;
				if (passPhysicalIndex != RenderPassInst::Unused)
				{
					if (bePhysicalPassUsed[res.mPhysicalIndex] != RenderPassInst::Unused &&
						passPhysicalIndex != bePhysicalPassUsed[res.mPhysicalIndex])
					{
						mPhysicalResourceDimensions[res.mPhysicalIndex].mIsTransient = false;
						break;
					}
					bePhysicalPassUsed[res.mPhysicalIndex] = passPhysicalIndex;
				}
			}
		}
	}

	void RenderGraphImpl::BuildRenderPassInfos()
	{
		for(PhysicalRenderPass& pass : mPhysicalRenderPasses)
		{
			
		}
	}

	void RenderGraphImpl::BuildBarriers()
	{
	}

	void RenderGraphImpl::BuildAliases()
	{
	}

	bool RenderGraphImpl::CheckPassDependent(I32 srcPass, I32 dstPass) const
	{
		return false;
	}

	bool RenderGraphImpl::Compile()
	{
		PROFILE_CPU_BLOCK("RenderGraphCompile");

		// clear datas
		mPassStack.clear();
		mExecuteRenderPasses.clear();
		mExecuteCmds.clear();
		mNeededResources.clear();

		// clear dependencies
		mPassDependencies.clear();
		mPassDependencies.resize(mRenderPasses.size());
		mPassMergeDependencies.clear();
		mPassMergeDependencies.resize(mRenderPasses.size());

		// check final res
		auto it = mResourceIndexMap.find(mFinalRes);
		if (it == nullptr)
		{
			Logger::Error("[RenderGraph] The final resource dose not exist.");
			return false;
		}
		ResourceInst& finalRes = mResources[*it];
		if (finalRes.mWrittenPasses.empty())
		{
			Logger::Error("[RenderGraph] The final resource dose not written by any passes.");
			return false;
		}

		// get all passes by traverse dependencies
		for (auto& passIndex : finalRes.mWrittenPasses) {
			mPassStack.push(passIndex);
		}
		auto tempStack = mPassStack;
		for (auto& passIndex : tempStack) {
			TraverseRenderPassDependency(mRenderPasses[passIndex], 0);
		}
		FilterRenderPass(mPassStack);

		// reorder passes
		ReorderRenderPass(mPassStack);

		// build physical resources
		BuildPhysicalResources();

		// build real passese
		BuildPhysicalPasses();

		// build transients
		BuildTransients();

		// build barriers
		BuildBarriers();

		// build aliases
		BuildAliases();

		// create frame bindingSet
		CreateFrameBindingSets();

		return true;
	}

	bool RenderGraphImpl::Execute()
	{
		PROFILE_CPU_BLOCK("RenderGraphExecute");
		
		// execute all renderPasses by jobsystem
		I32 renderPassCount = mPhysicalRenderPasses.size();

		mRenderPassExecuteStates.clear();
		mRenderPassExecuteStates.resize(renderPassCount);

		DynamicArray<JobSystem::JobInfo> passJobs;
		passJobs.reserve(renderPassCount);
		JobSystem::JobHandle jobHandle = JobSystem::INVALID_HANDLE;

		for(I32 i = 0; i < renderPassCount; i++)
		{
			bool needExecute = true;
			if (!needExecute)
			{	
				continue;
			}

			// setup 
			mRenderPassExecuteStates[i].mActive = true;
		}

		// execute render passes
		for (int i = 0; i < renderPassCount; i++)
		{
			RenderPassExecuteState& state = mRenderPassExecuteStates[i];
			if (!state.mActive) {
				continue;
			}

			JobSystem::RunJob([&](I32 param, void* data) 
			{
				RenderGraphImpl* impl = reinterpret_cast<RenderGraphImpl*>(data);
				if (!impl) {
					return;
				}

				RenderPassInst* renderPassInst = impl->mExecuteRenderPasses[param];
				state.mCmd = GPU::CreateCommandlist();

				state.emitPreBarriers();
				{	
					// execute pass
					auto ent = state.mCmd->Event(renderPassInst->mName);
					RenderGraphResources resources(*impl, renderPassInst->mRenderPass);
					renderPassInst->mRenderPass->Execute(resources, *state.mCmd);
				}
				state.emitPostBarriers();

				// compile cmd
				if (!state.mCmd->GetCommands().empty())
				{
					if (!GPU::CompileCommandList(*state.mCmd))
					{
						Concurrency::AtomicIncrement(&impl->mCompileFailCount);
						Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
					}
				}
			}, this, &jobHandle);
		}
		JobSystem::RunJobs(passJobs.data(), passJobs.size(), &jobHandle);

		// submit commands
		JobSystem::RunJob([&](I32 param, void* data) {
			RenderGraphImpl* impl = reinterpret_cast<RenderGraphImpl*>(data);
			if (!impl) {
				return;
			}

			if (impl->mCompileFailCount > 0) {
				return;
			}

			// submit all cmd of render pass
			DynamicArray<GPU::CommandList*> cmdsToSubmit;
			for (int i = 0; i < renderPassCount; i++)
			{
				GPU::CommandList* cmd = impl->mExecuteCmds[i];
				if (!cmd->GetCommands().empty()) {
					cmdsToSubmit.push(cmd);
				}
			}

			if (!GPU::SubmitCommandList(Span(cmdsToSubmit.data(), cmdsToSubmit.size())))
			{
				Logger::Warning("Failed to submit command lists.");
				return;
			}
        
        }, this, &jobHandle);

		return true;
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

	void RenderGraph::AddRenderPass(const char* name, RenderGraphQueueFlag queueFlag, RenderPass* renderPass)
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

	bool RenderGraph::Compile()
	{
		return mImpl->Compile();
	}

	bool RenderGraph::Execute()
	{
		return mImpl->Execute();
	}

	void RenderGraph::SetFinalResource(const RenderGraphResource& res)
	{
		// It is must be called before Compile()
		if (!mImpl->mFinalRes.IsEmpty()) {
			Logger::Warning("[RenderGraph] The final res is already set and is will be overlaped.");
		}
		mImpl->mFinalRes = res;
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
		mImpl->mResourceIndexMap.clear();

		mImpl->mAllocator.Reset();
	}

	String RenderGraph::ExportGraphviz()
	{
		return String();
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