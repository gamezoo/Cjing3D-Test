#pragma once

#include "core\common\definitions.h"

namespace Cjing3D
{
namespace Profiler
{
	void Initialize();
	void Uninitilize();

	void BeginFrame();
	void EndFrame();

	void BeginCPUBlock(const char* name);
	void EndCPUBlock();

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