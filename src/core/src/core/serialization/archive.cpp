#include "archive.h"
#include "core\filesystem\filesystem.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
	ArchiveBase::ArchiveBase(const String& path, ArchiveMode mode, BaseFileSystem* fileSystem) :
		mFilePath(path),
		mMode(mode),
		mFileSystem(fileSystem)
	{
	}

	ArchiveBase::~ArchiveBase()
	{
		Close();
	}

	void ArchiveBase::SetPath(const char* path)
	{
		mFilePath = path;
	}

	bool ArchiveBase::IsOpen() const
	{
		return mDataBuffer != nullptr;
	}

	void ArchiveBase::Close()
	{
		CJING_SAFE_DELETE_ARR(mDataBuffer, mDataSize);
	}

	bool ArchiveBase::Load(const String& path)
	{
		if (!mFileSystem) {
			return false;
		}

		if (!mFileSystem->IsFileExists(path)) {
			return false;
		}

		U32 dataSize = 0;
		if (!mFileSystem->ReadFile(path.c_str(), &mDataBuffer, dataSize)) 
		{
			Logger::Warning("Fail to open file:%s", path.c_str());
			return false;
		}

		mDataSize = dataSize;
		return true;
	}

	bool ArchiveBase::Save(const String& path)
	{
		if (path.empty()) {
			return false;
		}

		return mFileSystem->WriteFile(path.c_str(), mDataBuffer, static_cast<size_t> (mDataSize));
	}

	String ArchiveBase::GetDirectory() const
	{
		String path(Path::MAX_PATH_LENGTH, '\0');
		Path::GetPathParentPath(mFilePath, path.toSpan());
		return path;
	}
}