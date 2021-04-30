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
	// Definitions
	////////////////////////////////////////////////////////////////////////////////////////////

	struct AliasTransfer
	{
		U32 mFrom;
		U32 mTo;
	};
	
	// render pass internal inst
	struct RenderPassInst
	{
		enum { Unused = ~0u };

		StaticString<64> mName;
		RenderPass* mRenderPass = nullptr;
		I32 mIndex = 0;
		U32 mPhysicalIndex = Unused;

		~RenderPassInst()
		{
			if (mRenderPass != nullptr) {
				mRenderPass->~RenderPass();
			}
		}
	};

	// real available render pass
	struct PhysicalRenderPass
	{
		GPU::RenderPassInfo mRenderPassInfo;
		DynamicArray<I32> mSubPasses;
		DynamicArray<AliasTransfer> mAliasTransfer;
	};

	// resource internal inst
	struct ResourceInst
	{
		enum { Unused = ~0u };

		I32 mIndex = -1;
		StaticString<64> mName;
		U32 mPhysicalIndex = Unused;
		RenderGraphQueueFlags mQueueFlags = 0;
		DynamicArray<I32> mWrittenPasses;
		DynamicArray<I32> mReadPasses;
		GPU::ResHandle mImportedHandle;
		GPU::ResourceType mType;
		GPU::TextureDesc mTexDesc;
		GPU::BufferDesc mBufferDesc;
	};

	RenderGraphResourceDimension CreateResDimension(const ResourceInst& res)
	{
		RenderGraphResourceDimension dim;
		dim.mName = res.mName;
		dim.mType = res.mType;
		dim.mResIndex = res.mIndex;
		dim.mIsImported = res.mImportedHandle != GPU::ResHandle::INVALID_HANDLE;
	
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

	// render pass execute state
	struct RenderPassExecuteState
	{
		GPU::CommandList* mCmd = nullptr;
		GPU::CommandListType mType = GPU::COMMAND_LIST_GRAPHICS;
		bool mActive = false;
		bool mIsGraphics = true;

		void emitPreBarriers()
		{
		}

		void emitPostBarriers()
		{
		}
	};

	// resource timeline of render passes
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

	// barrier
	struct Barrier
	{
		U32 mResPhysicalIndex = 0;
	};

	// barriers
	struct Barriers
	{
		DynamicArray<Barrier> mInvalidate;
		DynamicArray<Barrier> mFlush;
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	// Resource cache
	////////////////////////////////////////////////////////////////////////////////////////////

	// resource cache, handle resources by ref count
	struct ResourceCache
	{
		struct BufferPayload
		{
			GPU::BufferDesc mDesc;
			GPU::ResHandle mHandle;
			bool mIsUsed = false;
		};
		DynamicArray<BufferPayload> mBuffers;

		struct TexturePayload
		{
			GPU::TextureDesc mDesc;
			GPU::ResHandle mHandle;
			bool mIsUsed = false;
		};
		DynamicArray<TexturePayload> mTextures;

		void Resize(U32 newSize)
		{
			mBuffers.resize(newSize);
			mTextures.resize(newSize);
		}

		void ClearUnusedRes()
		{
			// clear useless resources
			{
				auto it = mBuffers.begin();
				while (it != mBuffers.end())
				{
					if (!it->mIsUsed)
					{
						if (it->mHandle != GPU::ResHandle::INVALID_HANDLE) {
							GPU::DestroyResource(it->mHandle);
						}
						it = mBuffers.erase(it);
					}
					else {
						it++;
					}
				}
			}
			{
				auto it = mTextures.begin();
				while (it != mTextures.end())
				{
					if (!it->mIsUsed)
					{
						if (it->mHandle != GPU::ResHandle::INVALID_HANDLE) {
							GPU::DestroyResource(it->mHandle);
						}
						it = mTextures.erase(it);
					}
					else {
						it++;
					}
				}
			}
		}

		void Clear()
		{
			for (auto& buffer : mBuffers) {
				buffer.mIsUsed = false;
			}
			for (auto& texture : mTextures) {
				texture.mIsUsed = false;
			}
		}

		void Reset()
		{
			for (auto& buffer : mBuffers)
			{
				if (buffer.mHandle != GPU::ResHandle::INVALID_HANDLE) {
					GPU::DestroyResource(buffer.mHandle);
				}
			}
			mBuffers.clear();

			for (auto& texture : mTextures)
			{
				if (texture.mHandle != GPU::ResHandle::INVALID_HANDLE) {
					GPU::DestroyResource(texture.mHandle);
				}
			}
			mTextures.clear();
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	// RenderGraphImpl
	////////////////////////////////////////////////////////////////////////////////////////////

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
			mResourceCache.Reset();
		}

		void Clear();
		bool Compile();
		bool Execute(JobSystem::JobHandle& jobHandle);

		void FilterRenderPass(DynamicArray<I32>& renderPasses);
		void ReorderRenderPass(DynamicArray<I32>& renderPasses);
		void BuildPhysicalResources();
		void BuildPhysicalPasses();
		void BuildTransients();
		void BuildRenderPassInfos();
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

		void ExecuteGraphicsCommands(PhysicalRenderPass& pass, RenderPassExecuteState& state);
		void ExecuteComputeCommands(PhysicalRenderPass& pass, RenderPassExecuteState& state);

	public:

		GPU::ResHandle GetTexture(const RenderGraphResource& res, GPU::TextureDesc* outDesc = nullptr)
		{
			const auto& resInst = mResources[res.mIndex];
			Debug::CheckAssertion(resInst.mType == GPU::RESOURCETYPE_TEXTURE);
			if (resInst.mPhysicalIndex == ResourceInst::Unused) {
				return GPU::ResHandle::INVALID_HANDLE;
			}
			if (outDesc != nullptr) {
				*outDesc = resInst.mTexDesc;
			}
			return mResourceCache.mTextures[resInst.mPhysicalIndex].mHandle;
		}

		GPU::ResHandle GetBuffer(const RenderGraphResource& res, GPU::BufferDesc* outDesc = nullptr)
		{
			const auto& resInst = mResources[res.mIndex];
			Debug::CheckAssertion(resInst.mType == GPU::RESOURCETYPE_BUFFER);
			if (resInst.mPhysicalIndex == ResourceInst::Unused) {
				return GPU::ResHandle::INVALID_HANDLE;
			}
			if (outDesc != nullptr) {
				*outDesc = resInst.mBufferDesc;
			}
			return mResourceCache.mBuffers[resInst.mPhysicalIndex].mHandle;
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

		if (false)
		{
			// TODO:   
			auto CheckMerge = [&](const RenderPassInst& prev, const RenderPassInst& next) {
				return false;
			};
			PhysicalRenderPass physicalRenderPass;
			for (I32 index = 0; index < mPassIndexStack.size();)
			{
				I32 mergeEnd = index + 1;
				for (; mergeEnd < mPassIndexStack.size(); mergeEnd++)
				{
					physicalRenderPass.mSubPasses.push(mPassIndexStack[mergeEnd - 1]);

					bool mergeEnable = true;
					for (I32 mergeIndex = index; mergeIndex < mergeEnd; mergeIndex++)
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
		}
		else
		{
			PhysicalRenderPass physicalRenderPass;
			for (I32 index = 0; index < mPassIndexStack.size(); index++)
			{
				physicalRenderPass.mSubPasses.push(mPassIndexStack[index]);
				mPhysicalRenderPasses.push(std::move(physicalRenderPass));
			}
		}

		for (U32 i = 0; i < mPhysicalRenderPasses.size(); i++)
		{
			for (I32 passIndex : mPhysicalRenderPasses[i].mSubPasses) {
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

			if (dim.mIsImported) {
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

			if (res.mImportedHandle != GPU::ResHandle::INVALID_HANDLE) {
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

	void RenderGraphImpl::BuildRenderPassInfos()
	{
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
			for (I32 i = 0; i < physicalPass.mSubPasses.size(); i++, barrierIndex)
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
		mResourceCache.Clear();
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

		// build render pass infos
		BuildRenderPassInfos();

		// build barriers
		BuildBarriers();

		// build physical barriers
		BuildPhysicalBarriers();

		// build aliases
		BuildAliases();

		return true;
	}

	void RenderGraphImpl::ExecuteGraphicsCommands(PhysicalRenderPass& pass, RenderPassExecuteState& state)
	{
		GPU::CommandList& cmd = *state.mCmd;
		cmd.BeginRenderPass(pass.mRenderPassInfo);

		for (const I32& subPass : pass.mSubPasses)
		{
			RenderPassInst* renderPassInst = &mRenderPasses[subPass];

			// execute pass
			auto ent = cmd.Event(renderPassInst->mName);
			RenderGraphResources resources(*this, renderPassInst->mRenderPass);
			renderPassInst->mRenderPass->Execute(resources, cmd);
		}

		cmd.EndRenderPass();

		// compile cmd
		if (!state.mCmd->GetCommands().empty())
		{
			if (!GPU::CompileCommandList(cmd))
			{
				Concurrency::AtomicIncrement(&mCompileFailCount);
				Logger::Warning("Failed to compile cmd for render pass \"%s\"", pass);
			}
		}
	}

	void RenderGraphImpl::ExecuteComputeCommands(PhysicalRenderPass& pass, RenderPassExecuteState& state)
	{
		Debug::CheckAssertion(pass.mSubPasses.size() == 1);
		RenderPassInst* renderPassInst = &mRenderPasses[0];

		// execute pass
		auto ent = state.mCmd->Event(renderPassInst->mName);
		RenderGraphResources resources(*this, renderPassInst->mRenderPass);
		renderPassInst->mRenderPass->Execute(resources, *state.mCmd);

		// compile cmd
		if (!state.mCmd->GetCommands().empty())
		{
			if (!GPU::CompileCommandList(*state.mCmd))
			{
				Concurrency::AtomicIncrement(&mCompileFailCount);
				Logger::Warning("Failed to compile cmd for render pass \"%s\"", renderPassInst->mName.c_str());
			}
		}
	}

	bool RenderGraphImpl::Execute(JobSystem::JobHandle& jobHandle)
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

		JobSystem::JobHandle executeJobHandle = JobSystem::INVALID_HANDLE;
		for(I32 i = 0; i < renderPassCount; i++)
		{
			if (mPhysicalRenderPasses[i].mSubPasses.empty()) {
				continue;
			}

			mRenderPassExecuteStates[i].mActive = true;

			RenderGraphQueueFlags flag = mRenderPasses[mPhysicalRenderPasses[i].mSubPasses[0]].mRenderPass->GetQueueFlags();
			switch (flag)
			{
			case RENDER_GRAPH_QUEUE_GRAPHICS_BIT:
				mRenderPassExecuteStates[i].mIsGraphics = true;
				mRenderPassExecuteStates[i].mType = GPU::COMMAND_LIST_GRAPHICS;
				break;
			case RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT:
				mRenderPassExecuteStates[i].mIsGraphics = false;
				mRenderPassExecuteStates[i].mType = GPU::COMMAND_LIST_ASYNC_COMPUTE;
				break;
			default:
				Logger::Warning("Unsupport render graph queue flag:%d.", flag);
				break;
			}
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
				state.mCmd = GPU::CreateCommandlist(state.mType);
				state.emitPreBarriers();
				{	
					// graphics commands
					if (state.mIsGraphics) {
						ExecuteGraphicsCommands(physicalPass, state);
					}
					// compute commands
					else {
						ExecuteComputeCommands(physicalPass, state);
					}
				}
				state.emitPostBarriers();

			}, this, &executeJobHandle, StaticString<32>().Sprintf("Execute render pass:%d", i).c_str());
		}
		
		// submit commands (sequence submit)
		JobSystem::RunJobEx([&](I32 param, void* data) {
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
        
        }, this, &jobHandle, executeJobHandle, "SubmitCommands");

		return true;
	}

	void RenderGraphImpl::CreateBuffer(I32 physicalIndex)
	{
		auto& dim = mPhysicalResourceDimensions[physicalIndex];
		bool needCreate = true;
		if (mResourceCache.mBuffers[physicalIndex].mHandle != GPU::ResHandle::INVALID_HANDLE)
		{
			if (dim.mPersistent &&
				mResourceCache.mBuffers[physicalIndex].mDesc == dim.mBufferDesc) {
				needCreate = false;
			}
		}

		if (needCreate) 
		{
			mResourceCache.mBuffers[physicalIndex].mHandle = GPU::CreateBuffer(&dim.mBufferDesc, nullptr, dim.mName.c_str());
			mResourceCache.mBuffers[physicalIndex].mDesc = dim.mBufferDesc;
		}

		mResourceCache.mBuffers[physicalIndex].mIsUsed = true;
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
		if (mResourceCache.mTextures[physicalIndex].mHandle != GPU::ResHandle::INVALID_HANDLE)
		{
			if (dim.mPersistent &&
				mResourceCache.mTextures[physicalIndex].mDesc == dim.mTexDesc) {
				needCreate = false;
			}
		}

		if (needCreate) 
		{
			mResourceCache.mTextures[physicalIndex].mHandle = GPU::CreateTexture(&dim.mTexDesc, nullptr, dim.mName.c_str());
			mResourceCache.mTextures[physicalIndex].mDesc = dim.mTexDesc;
		}

		mResourceCache.mTextures[physicalIndex].mIsUsed = true;
	}

	void RenderGraphImpl::CreateResources()
	{
		U32 dimSize = mPhysicalResourceDimensions.size();
		mResourceCache.mBuffers.resize(dimSize);

		for (U32 i = 0; i < dimSize; i++)
		{
			RenderGraphResourceDimension& dim = mPhysicalResourceDimensions[i];
			if (dim.mType == GPU::ResourceType::RESOURCETYPE_BUFFER) {
				CreateBuffer(i);
			}
			else
			{
				if (dim.mIsTransient)
				{
					mResourceCache.mTextures[i].mHandle = GPU::CreateTransientTexture(&dim.mTexDesc);
					mResourceCache.mTextures[i].mDesc = dim.mTexDesc;
				}
				else if (dim.mIsImported)
				{

				}
				else 
				{
					CreateTexture(i);
				}
			}
		}

		mResourceCache.ClearUnusedRes();
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

	void RenderGraphResBuilder::ReadTexture(RenderGraphResource res)
	{
		if (!res) {
			return;
		}

		auto& resInst = mImpl.mResources[res.mIndex];
		if (resInst.mType != GPU::ResourceType::RESOURCETYPE_TEXTURE) {
			return;
		}

		resInst.mQueueFlags |= mRenderPass->GetQueueFlags();
		resInst.mTexDesc.mBindFlags |= GPU::BIND_SHADER_RESOURCE;
		resInst.mReadPasses.push(mRenderPass->GetIndex());
		mRenderPass->AddInput(res);
	}

	void RenderGraphResBuilder::WriteTexture(RenderGraphResource res)
	{
		if (!res) {
			return;
		}

		auto& resInst = mImpl.mResources[res.mIndex];
		if (resInst.mType != GPU::ResourceType::RESOURCETYPE_TEXTURE) {
			return;
		}

		resInst.mQueueFlags |= mRenderPass->GetQueueFlags();
		resInst.mTexDesc.mBindFlags |= GPU::BIND_UNORDERED_ACCESS;
		resInst.mWrittenPasses.push(mRenderPass->GetIndex());
		mRenderPass->AddOutput(res);
	}

	void RenderGraphResBuilder::ReadBuffer(RenderGraphResource res)
	{
		if (!res) {
			return;
		}

		auto& resInst = mImpl.mResources[res.mIndex];
		if (resInst.mType != GPU::ResourceType::RESOURCETYPE_BUFFER) {
			return;
		}

		resInst.mQueueFlags |= mRenderPass->GetQueueFlags();
		resInst.mBufferDesc.mBindFlags |= GPU::BIND_SHADER_RESOURCE;
		resInst.mReadPasses.push(mRenderPass->GetIndex());
		mRenderPass->AddInput(res);
	}

	void RenderGraphResBuilder::WriteBuffer(RenderGraphResource res)
	{
		if (!res) {
			return;
		}

		auto& resInst = mImpl.mResources[res.mIndex];
		if (resInst.mType != GPU::ResourceType::RESOURCETYPE_BUFFER) {
			return;
		}

		resInst.mQueueFlags |= mRenderPass->GetQueueFlags();
		resInst.mBufferDesc.mBindFlags |= GPU::BIND_UNORDERED_ACCESS;
		resInst.mWrittenPasses.push(mRenderPass->GetIndex());
		mRenderPass->AddOutput(res);
	}

	void RenderGraphResBuilder::AddRTV(RenderGraphResource res, RenderGraphAttachment attachment)
	{
		if (!res) {
			return;
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
		resource.mWrittenPasses.push(mRenderPass->GetIndex());

		// renderPass add rtv
		mRenderPass->AddRTV(res, attachment);
		mRenderPass->AddOutput(res);
	}

	void RenderGraphResBuilder::SetDSV(RenderGraphResource res, RenderGraphAttachment attachment)
	{
		if (!res) {
			return;
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
		resource.mWrittenPasses.push(mRenderPass->GetIndex());

		// renderPass add dsv
		mRenderPass->SetDSV(res, attachment);
		mRenderPass->AddOutput(res);
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

	bool RenderGraph::Execute(JobSystem::JobHandle& jobHandle)
	{
		return mImpl->Execute(jobHandle);
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
#ifdef DEBUG
		String ret;
		ret += "digraph \"graph\" {\n";
		ret += "rankdir = LR\n";
		ret += "bgcolor = black\n";
		ret += "node [shape=rectangle, fontname=\"helvetica\", fontsize=10]\n\n";
		
		// passes nodes
		for (const auto& pass : mImpl->mRenderPasses)
		{
			ret += pass.mName;
			ret += " [label=\"";
			ret += pass.mName;
			ret += ", id: ";
			ret += std::to_string(pass.mIndex).c_str();
			ret += "\", ";
			ret += "style=filled, fillcolor=";
			ret += pass.mPhysicalIndex != RenderPassInst::Unused ? "darkorange" : "darkorange4";
			ret += "]\n";
		}
		// res nodes
		for (const auto& res : mImpl->mResources)
		{
			ret += res.mName;
			ret += " [label=\"";
			ret += res.mName;
			ret += ", id: ";
			ret += std::to_string(res.mIndex).c_str();
			ret += "\", ";
			ret += "style=filled, fillcolor=";
			ret += res.mPhysicalIndex != RenderPassInst::Unused ? "skyblue" : "skyblue4";
			ret += "]\n";
		}

		// edge
		for (auto& physicalPass : mImpl->mPhysicalRenderPasses)
		{
			for (U32 passIndex : physicalPass.mSubPasses)
			{
				auto& pass = mImpl->mRenderPasses[passIndex];
				auto inputs = pass.mRenderPass->GetInputs();
				for (auto& input : inputs)
				{
					if (!input.IsEmpty())
					{
						auto& res = mImpl->mResources[input.mIndex];
						if (res.mPhysicalIndex != ResourceInst::Unused) 
						{
							ret += res.mName;
							ret += "-> ";
							ret += pass.mName;
							ret += " [color=darkolivegreen2]\n";
						}
					}
				}

				// outputs
				auto outputs = pass.mRenderPass->GetOutputs();
				for (auto& output : outputs)
				{
					if (!output.IsEmpty())
					{
						auto& res = mImpl->mResources[output.mIndex];
						if (res.mPhysicalIndex != ResourceInst::Unused) 
						{
							ret += pass.mName;
							ret += "-> ";
							ret += res.mName;
							ret += " [color=red2]\n";
						}
					}
				}
			}
		}

		ret += "}\n";
		Logger::Print(ret);
		return ret;
#endif
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
				return RenderGraphResource(res.mIndex);
			}
		}
		return RenderGraphResource();
	}

	RenderGraphResource RenderGraph::ImportTexture(const char* name, GPU::ResHandle handle, const GPU::TextureDesc& desc)
	{
		if (auto res = mImpl->GetResource(name))
		{
			Logger::Warning("Render graph resource is already imported, %s.", name);
			return RenderGraphResource(res->mIndex);
		}

		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl->mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_TEXTURE;
		resInst.mTexDesc = desc;
		resInst.mImportedHandle = handle;

		mImpl->mResources.push(resInst);
		mImpl->mResourceNameMap.insert(name, resInst.mIndex);

		return RenderGraphResource(resInst.mIndex);
	}

	RenderGraphResource RenderGraph::ImportBuffer(const char* name, GPU::ResHandle handle, const GPU::BufferDesc& desc)
	{
		if (auto res = mImpl->GetResource(name))
		{
			Logger::Warning("Render graph resource is already imported, %s.", name);
			return RenderGraphResource(res->mIndex);
		}

		ResourceInst resInst;
		resInst.mName = name;
		resInst.mIndex = mImpl->mResources.size();
		resInst.mType = GPU::ResourceType::RESOURCETYPE_BUFFER;
		resInst.mBufferDesc = desc;
		resInst.mImportedHandle = handle;

		mImpl->mResources.push(resInst);
		mImpl->mResourceNameMap.insert(name, resInst.mIndex);

		return RenderGraphResource(resInst.mIndex);
	}
}