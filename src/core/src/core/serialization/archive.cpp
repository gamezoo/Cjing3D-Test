#include "archive.h"
#include "core\filesystem\filesystem.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
	ArchiveBase::ArchiveBase(const String& path, ArchiveMode mode) :
		mFilePath(path),
		mMode(mode)
	{
	}

	ArchiveBase::~ArchiveBase()
	{
		Close();
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
		if (!FileSystem::IsFileExists(mFilePath.c_str())) {
			return false;
		}

		U32 dataSize = 0;
		if (!FileSystem::ReadFileBytes(path.c_str(), &mDataBuffer, dataSize)) 
		{
			Debug::Warning("Fail to open file:%s", path.c_str());
			return false;
		}

		mDataSize = dataSize;
		return true;
	}

	bool ArchiveBase::Save(const String& path)
	{
		if (mFilePath.empty()) {
			return false;
		}

		return FileSystem::SaveFile(mFilePath.c_str(), mDataBuffer, static_cast<size_t> (mDataSize));
	}

	String ArchiveBase::GetDirectory() const
	{
		return Path::GetPathParentPath(mFilePath) + "/";
	}
}