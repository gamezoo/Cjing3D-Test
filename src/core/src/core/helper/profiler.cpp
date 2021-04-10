#include "profiler.h"
#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\concurrency\concurrency.h"
#include "core\concurrency\jobsystem.h"
#include "core\memory\linearAllocator.h"
#include "core\helper\timer.h"
#include "core\helper\stream.h"

#ifdef	PROFILER_REMOTERY_ENABLE
#include "remotery\lib\Remotery.h"
#endif

namespace Cjing3D
{
namespace Profiler
{
	static const size_t THREAD_CONTEXT_BUFFER_SIZE = 1024 * 256;

	/// ////////////////////////////////////////////////////////////////////////
	/// Impl

	// 基于线程的ProfilerContext
	struct ThreadLocalContext
	{
		StaticString<64> mThreadName;
		Concurrency::Mutex mMutex;
		Concurrency::ThreadID mThreadID = 0;
		LinearAllocator mAllocator;
		DynamicArray<const char*> mOpenBlockStack; // 当前未闭合的blockStack(未执行End)
		U32 mBufStart = 0;
		U32 mBufEnd = 0;
		bool mIsShowInProfiler = false;

		ThreadLocalContext()
		{
			mAllocator.Reserve(THREAD_CONTEXT_BUFFER_SIZE);
			mOpenBlockStack.reserve(64);
		}
	};

	struct ProfilerImpl
	{
	public:
		Concurrency::Mutex mMutex;
		DynamicArray<ThreadLocalContext*> mThreadContexts;
		bool mIsPaused = true;
		volatile I32 mFiberWaitCount = 0;

		// global context for Frame/GPU
		ThreadLocalContext mGlobalContext;

	public:
		ProfilerImpl() {}
		~ProfilerImpl()
		{
			for (auto ctx : mThreadContexts) {
				CJING_SAFE_DELETE(ctx);
			}
			mThreadContexts.clear();
		}

		bool IsPaused()const {
			return mIsPaused;
		}
		void SetPause(bool isPaused) {
			mIsPaused = isPaused;
		}

		ThreadLocalContext* GetThreadLocalContext()
		{
			thread_local ThreadLocalContext* ctx = [&]() 
			{
				ThreadLocalContext* newCtx = CJING_NEW(ThreadLocalContext);
				newCtx->mThreadID = Concurrency::GetCurrentThreadID();
				Concurrency::ScopedMutex lock(mMutex);
				mThreadContexts.push(newCtx);
				return newCtx;
			}();
			return ctx;
		}

		template<typename T>
		void RecordBlock(ThreadLocalContext& ctx, ProfileType type, const T& value)
		{
			if (IsPaused()) {
				return;
			}

#pragma pack(1)
			struct 
			{
				ProfileBlockHeader mHeader;
				T mValue;
			} newBlock;
#pragma pack()
			size_t blockSize = sizeof(newBlock);
			newBlock.mHeader.mSize = (U16)blockSize;
			newBlock.mHeader.mType = type;
			newBlock.mHeader.mTime = Timer::GetAbsoluteRawTime();
			newBlock.mValue = value;

			Concurrency::ScopedMutex lock(ctx.mMutex);
			// 基于ctx.mBufStart, ctx.mBufEnd循环使用AllocatorBuffer
			U8* mem = ctx.mAllocator.GetBuffer();
			size_t capacity = ctx.mAllocator.GetCapacity();

			while (blockSize + ctx.mBufEnd - ctx.mBufStart > capacity) 
			{
				const U16 size = (U16)mem[ctx.mBufStart % capacity];
				ctx.mBufStart += size;
			}

			U32 end = ctx.mBufEnd % capacity;
			if (capacity - end >= blockSize){
				Memory::Memcpy(mem + end, &newBlock, blockSize);
			}
			else
			{
				size_t leftSize = capacity - end;
				Memory::Memcpy(mem + end, &newBlock, leftSize);
				Memory::Memcpy(mem, ((U8*)&newBlock) + leftSize, blockSize - leftSize);
			}
			ctx.mBufEnd += blockSize;
		}

		void GetProfilerData(MemoryStream& stream, ThreadLocalContext& ctx)
		{
			Concurrency::ScopedMutex lock(ctx.mMutex);
			stream.WriteString(ctx.mThreadName);
			stream.Write(ctx.mThreadID);
			stream.Write(ctx.mBufStart);
			stream.Write(ctx.mBufEnd);
			stream.Write(ctx.mIsShowInProfiler);
			stream.Write((U32)ctx.mAllocator.GetCapacity());
			stream.Write(ctx.mAllocator.GetBuffer(), ctx.mAllocator.GetCapacity());
		}

		void GetProfilerData(MemoryStream& stream)
		{
			Concurrency::ScopedMutex lock(mMutex);
			stream.Write((U32)mThreadContexts.size());
			GetProfilerData(stream, mGlobalContext);
			for (auto ctx : mThreadContexts) {
				GetProfilerData(stream, *ctx);
			}
		}
	};

	/// ////////////////////////////////////////////////////////////////////////
	/// Profiler
	ProfilerImpl* gImpl = nullptr;

	void Initialize()
	{
		gImpl = CJING_NEW(ProfilerImpl);
	}

	bool IsInitialied()
	{
		return gImpl != nullptr;
	}

	void Uninitilize()
	{
		CJING_SAFE_DELETE(gImpl);
	}

	void SetCurrentThreadName(const char* name)
	{
		Debug::CheckAssertion(IsInitialied());
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		ctx->mThreadName = name;
	}

	bool IsPaused()
	{
		return gImpl->IsPaused();
	}

	void SetPause(bool isPaused)
	{
		gImpl->SetPause(isPaused);
	}

	void BeginFrame()
	{
#ifdef PROFILE_ENABLE
		Debug::CheckAssertion(IsInitialied());
#endif
	}

	void EndFrame()
	{
#ifdef PROFILE_ENABLE
		Debug::CheckAssertion(IsInitialied());
		gImpl->RecordBlock(gImpl->mGlobalContext, ProfileType::FRAME, 0);
#endif
	}

	void BeginCPUBlock(const char* name)
	{
#ifdef PROFILE_ENABLE
		Debug::CheckAssertion(IsInitialied());
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		ctx->mOpenBlockStack.push(name);
		gImpl->RecordBlock(*ctx, ProfileType::BEGIN_CPU, name);
#endif
	}

	void EndCPUBlock()
	{
#ifdef PROFILE_ENABLE
		Debug::CheckAssertion(IsInitialied());
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		if (!ctx->mOpenBlockStack.empty())
		{
			ctx->mOpenBlockStack.pop();
			gImpl->RecordBlock(*ctx, ProfileType::END_CPU, 0);
		}
#endif
	}

	void ColorBlock(const Color4& color)
	{
#ifdef PROFILE_ENABLE
		// set next block color
		Debug::CheckAssertion(IsInitialied());
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		gImpl->RecordBlock(*ctx, ProfileType::COLOR, color.GetRGBA());
#endif
	}

	FiberSwitchData BeginFiberWaitBlock(U32 jobHandle)
	{
#ifdef PROFILE_ENABLE
		Debug::CheckAssertion(IsInitialied());

		// record fiber block
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		FiberWaitRecord record;
		record.mID = Concurrency::AtomicIncrement(&gImpl->mFiberWaitCount);
		record.mJobHandle = jobHandle;
		gImpl->RecordBlock(*ctx, ProfileType::BEGIN_FIBER_WAIT, record);

		// record switch data
		FiberSwitchData switchData;
		switchData.mID = record.mID;
		switchData.mCount = ctx->mOpenBlockStack.size();
		Memory::Memcpy(switchData.mFiberOpenBlocks, ctx->mOpenBlockStack.data(), 
			std::min(switchData.mCount, (U32)ARRAYSIZE(switchData.mFiberOpenBlocks)) * sizeof(const char*));
		return switchData;
#endif
	}

	void EndFiberWaitBlock(U32 jobHandle, const FiberSwitchData& switchData)
	{
#ifdef PROFILE_ENABLE
		Debug::CheckAssertion(IsInitialied());

		// record fiber block
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		FiberWaitRecord record;
		record.mID = switchData.mID;;
		record.mJobHandle = jobHandle;
		gImpl->RecordBlock(*ctx, ProfileType::END_FIBER_WAIT, record);

		// continue switch blocks
		for (U32 i = 0; i < switchData.mCount; ++i)
		{
			if (i < ARRAYSIZE(switchData.mFiberOpenBlocks)) {
				BeginCPUBlock(switchData.mFiberOpenBlocks[i]);
			}
			else {
				BeginCPUBlock("N/A");
			}
		}
#endif
	}

	void BeforeFiberSwitch()
	{
#ifdef PROFILE_ENABLE
		// 在Fiber切换前，将所有blocks清空
		Debug::CheckAssertion(IsInitialied());
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		while (!ctx->mOpenBlockStack.empty())
		{
			ctx->mOpenBlockStack.pop();
			gImpl->RecordBlock(*ctx, ProfileType::END_CPU, 0);
		}
#endif
	}

	void ShowInProfiler(bool show)
	{
		Debug::CheckAssertion(IsInitialied());
		ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
		ctx->mIsShowInProfiler = show;
	}

	void GetProfilerData(MemoryStream& stream)
	{
		Debug::CheckAssertion(IsInitialied());
		gImpl->GetProfilerData(stream);
	}
}
}