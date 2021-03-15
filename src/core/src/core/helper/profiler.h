#pragma once

#include "core\common\definitions.h"

//  «∑Ò÷ß≥÷Remotery
// PROFILER_REMOTERY_ENABLE

namespace Cjing3D
{
	class MemoryStream;

namespace Profiler
{
	void Initialize();
	bool IsInitialied();
	void Uninitilize();

	void SetCurrentThreadName(const char* name);
	bool IsPaused();
	void SetPause(bool isPaused);
	void BeginFrame();
	void EndFrame();
	void BeginCPUBlock(const char* name);
	void EndCPUBlock();

	void GetProfilerData(MemoryStream& stream);

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
}

#define PROFILER_CPU_BLOCK(name) Profiler::ScopedCPUBlock scope(name);
}