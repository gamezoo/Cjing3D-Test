#pragma once

#include "core\common\common.h"
#include "core\filesystem\filesystem.h"

#include <string>
#include <vector>

namespace Cjing3D {

	enum class ArchiveMode
	{
		ArchiveMode_Read,
		ArchiveMode_Write
	};

	class ArchiveBase
	{
	public:
		ArchiveBase(const String& path, ArchiveMode mode, BaseFileSystem& fileSystem);
		~ArchiveBase();

		bool IsOpen()const;
		void Close();
		String GetDirectory()const;

		virtual void SetPath(const char* path);
		virtual bool Load(const String& path);
		virtual bool Save(const String& path);

	protected:
		ArchiveMode mMode = ArchiveMode::ArchiveMode_Read;
		String mFilePath;
		BaseFileSystem& mFileSystem;

		char* mDataBuffer = nullptr;
		U32 mDataSize = 0;
		U32 mReadPos = 0;
		U32 mCurrentArchiveVersion = 0;
	};
}