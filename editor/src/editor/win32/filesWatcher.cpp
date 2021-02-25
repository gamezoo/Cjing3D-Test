#ifdef CJING3D_PLATFORM_WIN32

#include "filesWatcher.h"
#include "core\concurrency\concurrency.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
	template <int N>
	static void CharToWChar(WCHAR(&out)[N], const char* in)
	{
		const char* c = in;
		WCHAR* cout = out;
		while (*c && c - in < N - 1)
		{
			*cout = *c;
			++cout;
			++c;
		}
		*cout = 0;
	}

	static void WCharToChar(Span<char> out, const WCHAR* in, int length)
	{
		int realSize = length / sizeof(WCHAR);
		for (int i = 0; i < realSize; i++) {
			out[i] = static_cast<char>(in[i]);
		}
		out[realSize] = '\0';
	}

	static int WatcherTaskFunc(void* data);

	class FilesWatcherImpl
	{
	public:
		char mPath[Path::MAX_PATH_LENGTH];
		Concurrency::Thread mTask;
		void* mDirHandle = nullptr;
		bool mIsExit = false;

		OVERLAPPED mOverlapped;
		char mBuffer[4096];
		DWORD mReceived;

		Signal<void(const char*)> OnFileChanged;

	public:
		FilesWatcherImpl(const char* path) :
			mTask(WatcherTaskFunc, this, 65536, "WatcherTaskThread")
		{
			CopyString(mPath, path);
		}

		~FilesWatcherImpl()
		{
			if (mDirHandle != nullptr)
			{
				CancelIoEx(mDirHandle, nullptr);
				CloseHandle(mDirHandle);
			}

			mIsExit = true;
			mTask.Join();
		}
	};

	static void CALLBACK NotifCompletion(DWORD status, DWORD transfered, LPOVERLAPPED over)
	{
		auto* impl = reinterpret_cast<FilesWatcherImpl*>(over->hEvent);
		if (status == ERROR_OPERATION_ABORTED)
		{
			impl->mIsExit = true;
			return;
		}
		if (transfered == 0) {
			return;
		}

		FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)impl->mBuffer;
		while (info)
		{
			switch (info->Action)
			{
			case FILE_ACTION_RENAMED_NEW_NAME:
			case FILE_ACTION_ADDED:
			case FILE_ACTION_MODIFIED:
			case FILE_ACTION_REMOVED:
			{
				char path[Path::MAX_PATH_LENGTH];
				WCharToChar(path, info->FileName, info->FileNameLength);
				impl->OnFileChanged(path);
			}
			break;
			}

			info = info->NextEntryOffset == 0 ? nullptr : (FILE_NOTIFY_INFORMATION*)(((char*)info) + info->NextEntryOffset);
		}
	}

	static int WatcherTaskFunc(void* data)
	{
		auto* impl = reinterpret_cast<FilesWatcherImpl*>(data);

		WCHAR wPath[Path::MAX_PATH_LENGTH];
		CharToWChar(wPath, impl->mPath);

		// get dir handle by createFile
		impl->mDirHandle = ::CreateFile(
			wPath,
			FILE_LIST_DIRECTORY,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			nullptr);
		if (impl->mDirHandle == nullptr || impl->mDirHandle == INVALID_HANDLE_VALUE) {
			return -1;
		}

		memset(&impl->mOverlapped , 0, sizeof(impl->mOverlapped));
		impl->mOverlapped.hEvent = impl;

		const DWORD filter =
			FILE_NOTIFY_CHANGE_SECURITY | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_ACCESS |
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_FILE_NAME;

		while (!impl->mIsExit)
		{
			BOOL status = ::ReadDirectoryChangesW(impl->mDirHandle,
				impl->mBuffer,
				sizeof(impl->mBuffer),
				TRUE,
				filter,
				&impl->mReceived,
				&impl->mOverlapped,
				&NotifCompletion);
			if (status == FALSE) break;
			SleepEx(INFINITE, TRUE);
		}
		return 0;
	}

	FilesWatcher::FilesWatcher(const char* path)
	{
		mImpl = CJING_NEW(FilesWatcherImpl)(path);
	}

	FilesWatcher::~FilesWatcher()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	Signal<void(const char*)>& FilesWatcher::GetOnFilesChanged()
	{
		return mImpl->OnFileChanged;
	}
}

#endif