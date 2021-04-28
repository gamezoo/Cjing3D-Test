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


		bool operator==(const RenderGraphResourceDimension& a, const RenderGraphResourceDimension& b)
		{
			return a.mType == b.mType &&
				a.mIsTransient == b.mIsTransient &&
				a.mTexDesc == b.mTexDesc &&
				a.mBufferDesc == b.mBufferDesc;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Impl
	////////////////////////////////////////////////////////////////////////////////////////////
	struct AliasTransfer
	{
		U32 mFrom;
		U32 mTo;
	};
	
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
		DynamicArray<AliasTransfer> mAliasTransfer;
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
		RenderGraphQueueFlags mQueueFlags = 0;
		DynamicArray<I32> mWrittenPasses;
		DynamicArray<I32> mReadPasses;
	};

	RenderGraphResourceDimension CreateResDimension(const ResourceInst& res)
	{
		RenderGraphResourceDimension dim;
		dim.mType = res.mType;
	
		switch (res.mType)
		{
		case GPU::RESOURCETYPE_BUFFER:
			dim.mBufferDesc = res.mBufferDesc;
			break;
		case GPU::RESOURCETYPE_TEXTURE:
			dim.mTexDesc = res.mTexDesc;
			break;
		default:
			break;
		}
		
		return std::move(dim);
	}

	struct RenderPassExecuteState
	{
		GPU::CommandList* mCmd = nullptr;
		bool mActive = false;
		bool mIsGraphics = true;

		void emitPreBarriers()
		{
		}

		void emitPostBarriers()
		{
		}
	};

	struct ResPassRange
	{
		U32 mFirstReadPass = ~0;
		U32 mLastReadPass = 0;
		U32 mFirstWritePass = ~0;
		U32 mLastWritePass = 0;

		bool HasReader()const { return mFirstReadPass <= mLastReadPass; }
		bool HasWriter()const { return mFirstWritePass <= mLastWritePass; }
		bool IsEmpty()const { return !HasReader() && !HasWriter(); }
		bool CanAlias()const
		{
			if (HasReader() && HasWriter() && mFirstReadPass <= mFirstWritePass) {
				return false;
			}

			return true;
		}

		U32 LastUsedPass() const
		{
			U32 lastPass = 0;
			if (HasReader()) {
				lastPass = std::max(lastPass, mLastReadPass);
			}
			if (HasWriter()) {
				lastPass = std::max(lastPass, mLastWritePass);
			}
			return lastPass;
		}

		U32 FirstUsedPass() const
		{
			U32 firstReadPass = ~0u;
			if (HasReader()) {
				firstReadPass = std::min(firstReadPass, mFirstReadPass);
			}
			if (HasWriter()) {
				firstReadPass = std::min(firstReadPass, mFirstWritePass);
			}
			return firstReadPass;
		}

		bool CheckRangeOverlop(const ResPassRange& rhs)
		{
			if (IsEmpty() || rhs.IsEmpty()) {
				return false;
			}
			if (!CanAlias() || !rhs.CanAlias()) {
				return false;
			}

			return (LastUsedPass() >= rhs.FirstUsedPass()) &&
				   (rhs.LastUsedPass() >= FirstUsedPass());
		}
	};

	struct Barrier
	{
		U32 mResPhysicalIndex = 0;
	};

	struct Barriers
	{
		DynamicArray<Barrier> mInvalidate;
		DynamicArray<Barrier> mFlush;
	};

	struct ResourceCache
	{
		DynamicArray<GPU::BufferDesc> mBufferDescs;
		DynamicArray<GPU::ResHandle> mBuffers;

		DynamicArray<GPU::TextureDesc> mTextureDescs;
		DynamicArray<GPU::ResHandle> mTextures;

		void Clear()
		{
			for (auto& buffer : mBuffers) 
			{
				if (buffer != GPU::ResHandle::INVALID_HANDLE) {
					GPU::DestroyResource(buffer);
				}
			}
			mBuffers.clear();
		}
	};

	class RenderGraphImpl
	{
	public:
		LinearAllocator mAllocator;
		volatile I32 mCompileFailCount = 0;

		// render passes
		DynamicArray<RenderPassInst> mRenderPasses;
		HashMap<String, I32> mRenderPassIndexMap;

		DynamicArray<I32> mPassIndexStack;
		DynamicArray<Set<I32>> mPassIndexDependencies;
		DynamicArray<Set<I32>> mPassIndexMergeDependencies;

		// resource
		DynamicArray<ResourceInst> mResources;
		HashMap<StringID, I32> mResourceNameMap;
		ResourceCache mResourceCache;

		// barriers
		DynamicArray<Barriers> mPassBarriers;

		// aliases
		DynamicArray<U32> mPhysicalAliases;

		// physical res and passes
		RenderGraphResource mFinalRes;
		DynamicArray<RenderGraphResourceDimension> mPhysicalResourceDimensions;
		DynamicArray<PhysicalRenderPass> mPhysicalRenderPasses;

		// execute states
		DynamicArray<RenderPassExecuteState> mRenderPassExecuteStates;

	public:
		RenderGraphImpl() {
			mAllocator.Reserve(1024 * 1024);
		}
		~RenderGraphImpl() {
			mResourceCache.Clear();
		}

		void Clear();
		bool Compile();
		bool Execute();

		void FilterRenderPass(DynamicArray<I32>& renderPasses);
		void ReorderRenderPass(DynamicArray<I32>& renderPasses);
		void BuildPhysicalResources();
		void BuildPhysicalPasses();
		void BuildTransients();
		void BuildBarriers();
		void BuildPhysicalBarriers();
		void BuildAliases();

		void TraverseRenderPassDependency(const RenderPassInst& renderPass, U32 stackCount);
		void DependRenderPassRecursive(const RenderPassInst& renderPass, const DynamicArray<I32>& writtenPasses, U32 stackCount, bool mergeDependency);
		bool CheckPassDependent(I32 srcPass, I32 dstPass)const;

		void CreateBuffer(I32 physicalIndex);
		void CreateTexture(I32 physicalIndex);
		void CreateResources();
		void CreateFrameBindingSets();

	public:
		bool CreateResource(ResourceInst& resInst)
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

		ResourceInst* GetResource(const StringID& name)
		{
			auto it = mResourceNameMap.find(name);
			if (it == nullptr) {
				return nullptr;
			}
			return &mResources[*it];
		}
	};

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
		auto inputs = renderPass.mRenderPass->GetInputs();
		for (const RenderGraphResource& res : inputs)
		{
			if (!res.IsEmpty())
			{
				ResourceInst& resInst = mResources[res.mIndex];
				DependRenderPassRecursive(renderPass, resInst.mWrittenPasses, stackCount, false);
			}
		}
	}

	void RenderGraphImpl::DependRenderPassRecursive(const RenderPassInst& renderPass, const DynamicArray<I32>& writtenPasses, U32 stackCount, bool mergeDependency)
	{
		for (auto& passIndex : writtenPasses)
		{
			if (passIndex != renderPass.mIndex) {
				mPassIndexDependencies[renderPass.mIndex].insert(passIndex);
			}
		}

		if (mergeDependency)
		{
			for (auto& passIndex : writtenPasses)
			{
				if (passIndex != renderPass.mIndex) {
					mPassIndexMergeDependencies[renderPass.mIndex].insert(passIndex);
				}
			}
		}

		stackCount++;

		for (auto& passIndex : writtenPasses)
		{
			if (passIndex == renderPass.mIndex) {
				continue;
			}

			mPassIndexStack.push(passIndex);
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

		// 根据合并关系继续完善passDependencies, 所有依赖合并项都应依赖于当前项的依赖项
		for (I32 passIndex = 0; passIndex < mPassIndexMergeDependencies.size(); passIndex++)
		{
			auto& passDepIndexs = mPassIndexDependencies[passIndex];
			auto& passMergeDepIndexs = mPassIndexMergeDependencies[passIndex];

			for (auto mergeDepIndex : passMergeDepIndexs)
			{
				for (I32 depIndex : passDepIndexs)
				{
					if (mergeDepIndex == depIndex) {
						continue;
					}
					if (CheckPassDependent(mergeDepIndex, depIndex)) {
						continue;
					}

					mPassIndexDependencies[mergeDepIndex].insert(depIndex);
				}
			}
		}

		// 重新排序passStack,目标是减少"hard barrier"

		DynamicArray<I32> unscheduledPasses;
		unscheduledPasses.reserve(renderPasses.size());
		std::swap(unscheduledPasses, renderPasses);

		auto SchedulePass = [&](U32 index) {

			renderPasses.push(unscheduledPasses[index]);
			unscheduledPasses.erase(index);
		};
		SchedulePass(0);

		while (!unscheduledPasses.empty())
		{
			// 遍历所有未排序pass，找到最适合添加到当前队列后的pass
			// 1：合并项则直接添加
			// 2：否则找到与当前项目依赖关系最少的Pass 

			U32 bestFactor = 0;
			U32 bestIndex = 0;
			for (I32 index = 0; index < unscheduledPasses.size(); index++)
			{
				I32 unschedulePassIndex = unscheduledPasses[index];
				I32 currentFactor = 0;

				if (mPassIndexMergeDependencies[unschedulePassIndex].find(renderPasses.back()))
				{
					bestFactor = ~0u;
				}
				else
				{
					for (int j = renderPasses.size() - 1; j >= 0; j--)
					{
						if (CheckPassDependent(renderPasses[j], unscheduledPasses[index])) {
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
		for (auto& passIndex : mPassIndexStack)
		{
			// inputs
			RenderPassInst& renderPass = mRenderPasses[passIndex];
			auto inputs = renderPass.mRenderPass->GetInputs();
			for (auto& input : inputs)
			{
				if (!input.IsEmpty())
				{
					auto& res = mResources[input.mIndex];
					if (res.mPhysicalIndex == ResourceInst::Unused) {
						mPhysicalResourceDimensions.push(std::move(CreateResDimension(res)));
					}
					res.mPhysicalIndex++;
				}
			}

			// outputs
			auto outputs = renderPass.mRenderPass->GetOutputs();
			for (auto& output : outputs)
			{
				if (!output.IsEmpty())
				{
					auto& res = mResources[output.mIndex];
					if (res.mPhysicalIndex == ResourceInst::Unused) {
						mPhysicalResourceDimensions.push(std::move(CreateResDimension(res)));
					}
					res.mPhysicalIndex++;
				}
			}
		}
	}

	void RenderGraphImpl::BuildPhysicalPasses()
	{
		mPhysicalRenderPasses.clear();

		auto CheckMerge = [&](const RenderPassInst& prev, const RenderPassInst& next) {
			return false;
		};

		PhysicalRenderPass physicalRenderPass;
		for(I32 index = 0; index < mPassIndexStack.size();)
		{
			I32 mergeEnd = index + 1;
			for(; mergeEnd < mPassIndexStack.size(); mergeEnd++)
			{
				physicalRenderPass.mPasses.push(mPassIndexStack[mergeEnd - 1]);

				bool mergeEnable = true;
				for(I32 mergeIndex = index; mergeIndex < mergeEnd; mergeIndex++)
				{
					if (!CheckMerge(mRenderPasses[mPassIndexStack[mergeIndex]], mRenderPasses[mPassIndexStack[mergeEnd]]))
					{
						mergeEnable = false;
						break;
					}
				}
				if (!mergeEnable) {
					break;
				}
			}

			mPhysicalRenderPasses.push(std::move(physicalRenderPass));
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

			// buffer is never transient
			if (dim.mType == GPU::ResourceType::RESOURCETYPE_BUFFER) {
				dim.mIsTransient = false;
			}
		}

		// 如果一个资源被多个pass使用，则该pass不是transient
		for (auto& res : mResources)
		{
			if (res.mType != GPU::RESOURCETYPE_TEXTURE) {
				continue;
			}

			if (res.mPhysicalIndex == ResourceInst::Unused) {
				continue;
			}

			// check written passes
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

			// check read passes
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

	void RenderGraphImpl::BuildBarriers()
	{
		mPassBarriers.clear();
		mPassBarriers.reserve(mPassIndexStack.size());

		const auto GetBarrier = [&](DynamicArray<Barrier>& barriers, U32 index) -> Barrier& {
			auto itr = std::find_if(barriers.begin(), barriers.end(), [index](const Barrier& b) {
				return index == b.mResPhysicalIndex;
			});
			if (itr != barriers.end())
				return *itr;
			else
			{
				barriers.push({ index });
				return barriers.back();
			}
		};

		for (I32 index : mPassIndexStack)
		{
			auto& renderPass = mRenderPasses[index];
			Barriers barriers;

			// input resources
			auto inputs = renderPass.mRenderPass->GetInputs();
			for (auto& input : inputs)
			{
				if (!input.IsEmpty())
				{
					GetBarrier(barriers.mInvalidate, mResources[input.mIndex].mPhysicalIndex);
				}
			}

			// output resources
			auto outputs = renderPass.mRenderPass->GetOutputs();
			for (auto& output : outputs)
			{
				if (!output.IsEmpty())
				{
					GetBarrier(barriers.mFlush, mResources[output.mIndex].mPhysicalIndex);
				}
			}

			mPassBarriers.push(std::move(barriers));
		}
	}

	void RenderGraphImpl::BuildPhysicalBarriers()
	{
		I32 barrierIndex = 0;
		for (PhysicalRenderPass& physicalPass : mPhysicalRenderPasses)
		{
			for (I32 i = 0; i < physicalPass.mPasses.size(); i++, barrierIndex)
			{
				Barriers& barriers = mPassBarriers[barrierIndex];
				for (auto& invalidate : barriers.mInvalidate)
				{
					// ignore barriers for transients
					if (mPhysicalResourceDimensions[invalidate.mResPhysicalIndex].mIsTransient) {
						continue;
					}
				}

				for (auto& flush : barriers.mFlush)
				{
					// ignore barriers for transients
					if (mPhysicalResourceDimensions[flush.mResPhysicalIndex].mIsTransient) {
						continue;
					}
				}
			}
		}
	}

	void RenderGraphImpl::BuildAliases()
	{
		// 获取每个Res的Range(第一个使用的Pass和最后一个使用的Pass)，如果有相同Dim的Res
		// 且Range no overlap，则res可以Alias
		
		DynamicArray<ResPassRange> resPassRage(mPhysicalResourceDimensions.size());
		auto RegisterReader = [&resPassRage](const ResourceInst& res, U32 passIndex)
		{
			if (res.mPhysicalIndex != ResourceInst::Unused)
			{
				auto& range = resPassRage[res.mPhysicalIndex];
				range.mFirstReadPass = std::min(range.mFirstReadPass, passIndex);
				range.mLastReadPass  = std::max(range.mLastReadPass,  passIndex);
			}
		};
		auto RegisterWriter = [&resPassRage](const ResourceInst& res, U32 passIndex)
		{
			if (res.mPhysicalIndex != ResourceInst::Unused)
			{
				auto& range = resPassRage[res.mPhysicalIndex];
				range.mFirstWritePass = std::min(range.mFirstWritePass, passIndex);
				range.mLastWritePass = std::max(range.mLastWritePass, passIndex);
			}
		};

		// 1. get pass range for each physical res
		for (I32 index : mPassIndexStack)
		{
			auto& renderPass = mRenderPasses[index];

			// input resources
			auto inputs = renderPass.mRenderPass->GetInputs();
			for (auto& input : inputs)
			{
				if (!input.IsEmpty()) {
					RegisterReader(mResources[input.mIndex], renderPass.mPhysicalIndex);
				}
			}

			// output resources
			auto outputs = renderPass.mRenderPass->GetOutputs();
			for (auto& output : outputs)
			{
				if (!output.IsEmpty()){
					RegisterWriter(mResources[output.mIndex], renderPass.mPhysicalIndex);
				}
			}
		}

		// 2. clear physical aliases
		mPhysicalAliases.resize(mPhysicalResourceDimensions.size());
		for (U32& v : mPhysicalAliases) {
			v = ~0;
		}

		// 3. find available aliases
		DynamicArray<DynamicArray<U32>> aliasChains(mPhysicalResourceDimensions.size());
		for (U32 i = 0; i < mPhysicalResourceDimensions.size(); i++)
		{
			auto& dim = mPhysicalResourceDimensions[i];

			// no alias for buffer
			if (dim.mType == GPU::ResourceType::RESOURCETYPE_BUFFER) {
				continue;
			}

			for (U32 j = 0; j < i; j++)
			{
				if (mPhysicalResourceDimensions[j] == mPhysicalResourceDimensions[i])
				{
					if (!resPassRage[i].CheckRangeOverlop(resPassRage[j]))
					{
						mPhysicalAliases[i] = j;

						if (aliasChains[j].empty()) {
							aliasChains[j].push(j);
						}
						aliasChains[j].push(i);

						break;
					}
				}
			}
		}

		// 4. build alias transfer by alias chains
		for (auto& chain : aliasChains)
		{
			if (chain.empty()) {
				continue;
			}

			for (U32 i = 0; i < chain.size(); i ++)
			{
				if (i + 1 < chain.size()) {
					mPhysicalRenderPasses[chain[i]].mAliasTransfer.push({chain[i], chain[i + 1]});
				}
				else {
					mPhysicalRenderPasses[chain[i]].mAliasTransfer.push({ chain[i], chain[0] });
				}
			}
		}
	}

	bool RenderGraphImpl::CheckPassDependent(I32 srcPass, I32 dstPass) const
	{
		if (srcPass == dstPass) {
			return true;
		}

		for (I32 dep : mPassIndexDependencies[dstPass])
		{
			if (CheckPassDependent(srcPass, dep)) {
				return true;
			}
		}
		return false;
	}

	void RenderGraphImpl::Clear()
	{
		for (auto& renderPassInst : mRenderPasses) {
			renderPassInst.mRenderPass->~RenderPass();
		}

		// renderPasses
		mRenderPasses.clear();
		mRenderPassIndexMap.clear();

		// resources
		mResources.clear();
		mResourceNameMap.clear();
		mFinalRes = RenderGraphResource();

		mAllocator.Reset();
	}

	bool RenderGraphImpl::Compile()
	{
		PROFILE_CPU_BLOCK("RenderGraphCompile");

		// clear datas
		mPassIndexStack.clear();
		mPassIndexDependencies.clear();
		mPassIndexDependencies.resize(mRenderPasses.size());
		mPassIndexMergeDependencies.clear();
		mPassIndexMergeDependencies.resize(mRenderPasses.size());

		// check final res
		if (mFinalRes.IsEmpty())
		{
			Logger::Error("[RenderGraph] The final resource dose not exist.");
			return false;
		}
		ResourceInst& finalRes = mResources[mFinalRes.mIndex];
		if (finalRes.mWrittenPasses.empty())
		{
			Logger::Error("[RenderGraph] The final resource dose not written by any passes.");
			return false;
		}

		// get available passes by traversing pass dependencies
		for (auto& passIndex : finalRes.mWrittenPasses) {
			mPassIndexStack.push(passIndex);
		}
		auto tempStack = mPassIndexStack;
		for (auto& passIndex : tempStack) {
			TraverseRenderPassDependency(mRenderPasses[passIndex], 0);
		}
		FilterRenderPass(mPassIndexStack);

		// reorder passes
		ReorderRenderPass(mPassIndexStack);

		// build physical resources
		BuildPhysicalResources();

		// build real passese
		BuildPhysicalPasses();

		// build transients
		BuildTransients();

		// build barriers
		BuildBarriers();

		// build physical barriers
		BuildPhysicalBarriers();

		// build aliases
		BuildAliases();

		return true;
	}

	bool RenderGraphImpl::Execute()
	{
		PROFILE_CPU_BLOCK("RenderGraphExecute");

		// create resources
		CreateResources();

		// create frame bindingSet	
		CreateFrameBindingSets();

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

				PhysicalRenderPass& physicalPass = impl->mPhysicalRenderPasses[param];
				state.mCmd = GPU::CreateCommandlist();
				state.emitPreBarriers();
				{	
					// graphics commands
					if (state.mIsGraphics)
					{
						for (const I32& subPass : physicalPass.mPasses)
						{
							RenderPassInst* renderPassInst = &impl->mRenderPasses[subPass];

							// execute pass
							auto ent = state.mCmd->Event(renderPassInst->mName);
							RenderGraphResources resources(*impl, renderPassInst->mRenderPass);
							renderPassInst->mRenderPass->Execute(resources, *state.mCmd);

							// compile cmd
							if (!state.mCmd->GetCommands().empty())
							{
								if (!GPU::CompileCommandList(*state.mCmd))
								{
									Concurrency::AtomicIncrement(&impl->mCompileFailCount);
									Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
								}
							}
						}
					}
					// compute commands
					else
					{
						Debug::CheckAssertion(physicalPass.mPasses.size() == 1);
						RenderPassInst* renderPassInst = &impl->mRenderPasses[0];

						// execute pass
						auto ent = state.mCmd->Event(renderPassInst->mName);
						RenderGraphResources resources(*impl, renderPassInst->mRenderPass);
						renderPassInst->mRenderPass->Execute(resources, *state.mCmd);

						// compile cmd
						if (!state.mCmd->GetCommands().empty())
						{
							if (!GPU::CompileCommandList(*state.mCmd))
							{
								Concurrency::AtomicIncrement(&impl->mCompileFailCount);
								Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
							}
						}
					}
				}
				state.emitPostBarriers();

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
				GPU::CommandList* cmd = impl->mRenderPassExecuteStates[i].mCmd;
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

	void RenderGraphImpl::CreateBuffer(I32 physicalIndex)
	{
		auto& dim = mPhysicalResourceDimensions[physicalIndex];
		bool needCreate = true;
		if (mResourceCache.mBuffers[physicalIndex] != GPU::ResHandle::INVALID_HANDLE)
		{
			if (dim.mPersistent &&
				mResourceCache.mBufferDescs[physicalIndex] == dim.mBufferDesc) {
				needCreate = false;
			}
		}

		if (needCreate) 
		{
			mResourceCache.mBuffers[physicalIndex] = GPU::CreateBuffer(&dim.mBufferDesc, nullptr, dim.mName.c_str());
			mResourceCache.mBufferDescs[physicalIndex] = dim.mBufferDesc;
		}
	}

	void RenderGraphImpl::CreateTexture(I32 physicalIndex)
	{
		auto& dim = mPhysicalResourceDimensions[physicalIndex];

		// if current res is aliase
		if (mPhysicalAliases[physicalIndex] != ~0)
		{
			mResourceCache.mTextures[physicalIndex] = mResourceCache.mTextures[mPhysicalAliases[physicalIndex]];
			return;
		}

		bool needCreate = true;
		if (mResourceCache.mTextures[physicalIndex] != GPU::ResHandle::INVALID_HANDLE)
		{
			if (dim.mPersistent &&
				mResourceCache.mTextureDescs[physicalIndex] == dim.mTexDesc) {
				needCreate = false;
			}
		}

		if (needCreate) 
		{
			mResourceCache.mTextures[physicalIndex] = GPU::CreateTexture(&dim.mTexDesc, nullptr, dim.mName.c_str());
			mResourceCache.mTextureDescs[physicalIndex] = dim.mTexDesc;
		}
	}

	void RenderGraphImpl::CreateResources()
	{
		for (U32 i = 0; i < mPhysicalResourceDimensions.size(); i++)
		{
			RenderGraphResourceDimension& dim = mPhysicalResourceDimensions[i];
			if (dim.mType == GPU::ResourceType::RESOURCETYPE_BUFFER) {
				CreateBuffer(i);
			}
			else
			{
				if (dim.mIsTransient)
				{
					// allocate transient obj?
					mResourceCache.mTextures[i];
					mResourceCache.mTextureDescs[i] = dim.mTexDesc;
				}
				else 
				{
					CreateTexture(i);
				}
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
		if (auto res = mImpl.GetResource(name))
		{
			Logger::Warning("Render graph resource is already created, %s.", name);
			return RenderGraphResource(res->mIndex);
		}

		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl.mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_TEXTURE;
		resInst.mTexDesc = *desc;
		mImpl.mResources.push(resInst);
		mImpl.mResourceNameMap.insert(name, resInst.mIndex);

		return RenderGraphResource(resInst.mIndex);
	}

	RenderGraphResource RenderGraphResBuilder::CreateBuffer(const char* name, const GPU::BufferDesc* desc)
	{
		if (auto res = mImpl.GetResource(name)) 
		{
			Logger::Warning("Render graph resource is already created, %s.", name);
			return RenderGraphResource(res->mIndex);
		}

		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl.mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_BUFFER;
		resInst.mBufferDesc = *desc;
		mImpl.mResources.push(resInst);
		mImpl.mResourceNameMap.insert(name, resInst.mIndex);

		return RenderGraphResource(resInst.mIndex);
	}

	RenderGraphResource RenderGraphResBuilder::AddInput(RenderGraphResource res)
	{
		if (!res) {
			return res;
		}

		// res record pass index
		auto& resInst = mImpl.mResources[res.mIndex];
		resInst.mQueueFlags |= mRenderPass->GetQueueFlags();
		resInst.mReadPasses.push(mRenderPass->GetIndex());

		// update render pass
		mRenderPass->AddInput(res);
		return res;
	}

	RenderGraphResource RenderGraphResBuilder::AddOutput(RenderGraphResource res)
	{
		if (!res) {
			return res;
		}

		// res record pass index
		auto& resInst = mImpl.mResources[res.mIndex];
		resInst.mQueueFlags |= mRenderPass->GetQueueFlags();
		resInst.mWrittenPasses.push(mRenderPass->GetIndex());

		// update version for trace renderPass resource dependencies
		mRenderPass->AddInput(res);
		res.mVersion++;
		mRenderPass->AddOutput(res);
		return res;
	}

	RenderGraphResource RenderGraphResBuilder::AddRTV(RenderGraphResource res, RenderGraphAttachment attachment)
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

	RenderGraphResource RenderGraphResBuilder::SetDSV(RenderGraphResource res, RenderGraphAttachment attachment)
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
		CJING_SAFE_DELETE(mImpl);
	}

	void RenderGraph::AddRenderPass(const char* name, RenderGraphQueueFlag queueFlag, RenderPass* renderPass, RenderGraphResBuilder& builder)
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

		renderPass->SetIndex(index);
		renderPass->AddQueueFlag(queueFlag);

		// setup render pass after pass push into the graph
		renderPass->Setup(builder);
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
		if (!mImpl->mFinalRes.IsEmpty()) {
			Logger::Warning("[RenderGraph] The final res is already set and is will be overlaped.");
		}
		mImpl->mFinalRes = res;
	}

	void RenderGraph::Clear()
	{
		mImpl->Clear();
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