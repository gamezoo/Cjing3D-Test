#pragma once

#include "client\common\common.h"

namespace Cjing3D
{
	struct PresentConfig
	{
		I32x2  mScreenSize = 0;
		bool   mIsLockFrameRate = false;
		U32    mTargetFrameRate = 60;
		U32    mMultiSampleCount = 1;
		bool   mIsFullScreen = false;
	};
}