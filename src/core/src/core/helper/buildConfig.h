#pragma once

#include "core\string\string.h"

namespace Cjing3D
{
	class BaseFileSystem;

	namespace BuildConfig
	{
		void Initialize(BaseFileSystem* filesystem);
		String GetBuildCmd();
		String GetProfile();
	}
}