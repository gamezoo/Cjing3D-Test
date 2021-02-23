#pragma once

#include "core\common\common.h"
#include "core\signal\signal.h"

namespace Cjing3D
{
	class FilesWatcher
	{
	public:
		FilesWatcher(const char* path);
		~FilesWatcher();

		Signal<void(const char*)>& GetSignal();

	private:
		class FilesWatcherImpl* mImpl = nullptr;
	};
}