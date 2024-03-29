#pragma once

#include "core\common\common.h"
#include "core\container\span.h"

namespace Cjing3D
{
	enum InitConfigFlag
	{
		PresentFlag_WinApp     = 1 << 1,
		PresentFlag_ConsoleApp = 1 << 2
	};

	struct InitConfig
	{
		Span<const char*> mPlugins;
		const char* mTitle = "CJING3D";
		const char* mPackPath = nullptr;
		const char* mWorkPath = nullptr;
		I32x2  mScreenSize = 0;
		bool   mIsLockFrameRate = false;
		U32    mTargetFrameRate = 60;
		U32    mMultiSampleCount = 1;
		bool   mIsFullScreen = false;
		bool   mIsApp = false;
		I32    mFlag = 0;
	};
}