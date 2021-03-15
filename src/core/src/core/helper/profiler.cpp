#include "profiler.h"
#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\concurrency\concurrency.h"
#include "core\platform\platform.h"
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

	enum class ProfileType
	{
		BEGIN_CPU,
		END_CPU
	};

	/// ////////////////////////////////////////////////////////////////////////
	/// Impl
	template<typename T>
	struct ProfileBlock
	{
		size_t mSize;
		ProfileType mType;
		F32 mTime;
		T mValue;
	};

	struct ThreadLocalContext
	{
		StaticString<64> mThreadName;
		Concurrency::Mutex mMutex;
		Concurrency::ThreadID mThreadID = 0;
		LinearAllocator mAllocator;
		DynamicArray<const char*> mBlocks;
		U32 mBufStart = 0;
		U32 mBufEnd = 0;

		ThreadLocalContext()
		{
			mAllocator.Reserve(THREAD_CONTEXT_BUFFER_SIZE);
			mBlocks.reserve(64);
		}
	};

	struct ProfilerImpl
	{
	private:
		Concurrency::Mutex mMutex;
		DynamicArray<ThreadLocalContext*> mThreadContexts;
		ThreadLocalContext mGlobalContext;
		bool mIsPaused = true;

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
			ProfileBlock<T> newBlock;
			size_t blockSize = sizeof(newBlock);
			newBlock.mSize = blockSize;
			newBlock.mType = type;
			newBlock.mTime = Timer::GetAbsoluteTime();
			newBlock.mValue = value;

			Concurrency::ScopedMutex lock(ctx.mMutex);
			// 基于ctx.mBufStart, ctx.mBufEnd循环使用AllocatorBuffer
			U8* mem = ctx.mAllocator.GetBuffer();
			size_t capacity = ctx.mAllocator.GetCapacity();

			while (blockSize + ctx.mBufEnd - ctx.mBufStart > capacity) {
				ctx.mBufStart += mem[ctx.mBufStart % capacity];
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
			stream.Write((U32)ctx.mAllocator.GetCapacity());
			stream.Write(ctx.mAllocator.GetBuffer(), ctx.mAllocator.GetCapacity());
		}

		void GetProfilerData(MemoryStream& stream)
		{
			Concurrency::ScopedMutex lock(mMutex);
			stream.Write((U32)mThreadContexts.size());
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
		Debug::CheckAssertion(IsInitialied());
	}

	void EndFrame()
	{
		Debug::CheckAssertion(IsInitialied());
	}

	void BeginCPUBlock(const char* name)
	{
		Debug::CheckAssertion(IsInitialied());
		if (!gImpl->IsPaused())
		{
			ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
			ctx->mBlocks.push(name);
			gImpl->RecordBlock(*ctx, ProfileType::BEGIN_CPU, name);
		}
	}

	void EndCPUBlock()
	{
		Debug::CheckAssertion(IsInitialied());
		if (!gImpl->IsPaused())
		{
			ThreadLocalContext* ctx = gImpl->GetThreadLocalContext();
			if (!ctx->mBlocks.empty())
			{
				const char* name = ctx->mBlocks.back();
				ctx->mBlocks.pop();
				gImpl->RecordBlock(*ctx, ProfileType::END_CPU, name);
			}
		}
	}

	void GetProfilerData(MemoryStream& stream)
	{
		Debug::CheckAssertion(IsInitialied());
		gImpl->GetProfilerData(stream);
	}
}
}