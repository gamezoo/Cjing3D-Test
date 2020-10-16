#include "binaryArchive.h"
#include "core\filesystem\filesystem.h"

namespace Cjing3D
{
	const U32 BinaryArchive::currentArchiveVersion = 2;

	BinaryArchive::BinaryArchive(const String& path, ArchiveMode mode) :
		ArchiveBase(path, mode)
	{
		if (mMode == ArchiveMode::ArchiveMode_Read)
		{
			if (!Load(mFilePath.c_str())) {
				return;
			}

			if (mCurrentArchiveVersion == 0) {
				this->operator>>(mCurrentArchiveVersion);
			}

			if (mCurrentArchiveVersion != currentArchiveVersion) {
				Close();
			}
		}
		else
		{
			// 如果是写模式则首先先写入版本号
			mCurrentArchiveVersion = BinaryArchive::currentArchiveVersion;
			mDataSize = 128;
			mDataBuffer = new char[mDataSize];

			this->operator<<(mCurrentArchiveVersion);
		}
	}

	BinaryArchive::~BinaryArchive()
	{
		if (mMode == ArchiveMode::ArchiveMode_Write)
		{
			if (mDataBuffer != nullptr) {
				Save(mFilePath);
			}
		}
	}

	void BinaryArchive::WriteImpl(const void* data, U32 size)
	{
		// 如果新的大小大于分配大小，则重新分配2倍的大小
		U32 newSize = mReadPos + size;
		if (newSize > mDataSize)
		{
			U32 newBufferSize = newSize * 2;
			char* newBuffer = new char[newBufferSize];
			memcpy_s(newBuffer, newBufferSize, mDataBuffer, mDataSize);
	
			CJING_DELETE_ARR(mDataBuffer, mDataSize);

			mDataSize = newBufferSize;
			mDataBuffer = newBuffer;
		}

		memcpy_s(reinterpret_cast<void*>(mDataBuffer + mReadPos), mDataSize, data, size);
		mReadPos = newSize;
	}

	void BinaryArchive::ReadImpl(void* data, U32 size)
	{
		memcpy_s(data, size, reinterpret_cast<void*>(mDataBuffer + mReadPos), size);
		mReadPos += size;
	}
}