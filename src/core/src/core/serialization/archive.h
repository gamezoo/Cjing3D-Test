#pragma once

#include "core\common\common.h"

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
		ArchiveBase(const String& path, ArchiveMode mode);
		~ArchiveBase();

		bool IsOpen()const;
		void Close();
		String GetDirectory()const;

		virtual bool Load(const String& path);
		virtual bool Save(const String& path);

	protected:
		ArchiveMode mMode = ArchiveMode::ArchiveMode_Read;
		String mFilePath;

		char* mDataBuffer = nullptr;
		U32 mDataSize = 0;
		U32 mReadPos = 0;
		U32 mCurrentArchiveVersion = 0;
	};
}