#pragma once

#include "core\common\definitions.h"
#include "math\color.h"

// �Ƿ�֧��Remotery
// PROFILER_REMOTERY_ENABLE

namespace Cjing3D
{
	class MemoryStream;

namespace Profiler
{
#define PROFILE_ENABLE

	enum class ProfileType : U8
	{
		BEGIN_CPU,
		END_CPU,
		COLOR,
		BEGIN_FIBER_WAIT,
		END_FIBER_WAIT,
		FRAME
	};

	struct FiberWaitRecord
	{
		I32 mID;
		U32 mJobHandle;
	};

	struct FiberSwitchData 
	{
		I32 mID = 0;
		const char* mFiberOpenBlocks[16];
		U32 mCount = 0;
	};

#pragma pack(1)
	struct ProfileBlockHeader
	{
		U16 mSize;
		ProfileType mType;
		U64 mTime;
	};
#pragma pack()

	void Initialize();
	bool IsInitialied();
	void Uninitilize();

	void SetCurrentThreadName(const char* name);
	bool IsPaused();
	void SetPause(bool isPaused);
	void ShowInProfiler(bool show);
	void GetProfilerData(MemoryStream& stream);
	void BeforeFiberSwitch();

	void BeginFrame();
	void EndFrame();
	void BeginCPUBlock(const char* name);
	void EndCPUBlock();
	void ColorBlock(const Color4& color);
	FiberSwitchData BeginFiberWaitBlock(U32 jobHandle);
	void EndFiberWaitBlock(U32 jobHandle, const FiberSwitchData& switchData);

	struct ScopedCPUBlock
	{
		explicit ScopedCPUBlock(const char* name)
		{
			BeginCPUBlock(name);
		}

		~ScopedCPUBlock()
		{
			EndCPUBlock();
		}
	};

	struct ScopedFiberSwitchBlock
	{
		FiberSwitchData mSwitchData;
		U32 mJobHandle;

		ScopedFiberSwitchBlock(U32 jobHandle)
		{
			mSwitchData = BeginFiberWaitBlock(jobHandle);
			mJobHandle = jobHandle;
		}

		~ScopedFiberSwitchBlock()
		{
			EndFiberWaitBlock(mJobHandle, mSwitchData);
		}
	};
}

#define PROFILE_FILBER_SWITCH(jobHandle) Profiler::ScopedFiberSwitchBlock scopedSwitch(jobHandle);
#define PROFILE_FUNCTION() Profiler::ScopedCPUBlock scope(__FUNCTION__);
#define PROFILE_CPU_BLOCK(name) Profiler::ScopedCPUBlock scope(name);
}