#include "binaryArchive.h"
#include "core\filesystem\filesystem.h"

namespace Cjing3D
{
	const U32 BinaryArchive::currentArchiveVersion = 2;

	BinaryArchive::BinaryArchive(const String& path, ArchiveMode mode, BaseFileSystem& fileSystem) :
		ArchiveBase(path, mode, fileSystem)
	{
		OpenBinaryFile(path);
	}

	BinaryArchive::~BinaryArchive()
	{
		if (mMode == ArchiveMode::ArchiveMode_Write)
		{
			if (!mFilePath.empty() && mDataBuffer != nullptr) {
				Save(mFilePath);
			}
		}
	}

	void BinaryArchive::OpenBinaryFile(const char* path)
	{
		if (mMode == ArchiveMode::ArchiveMode_Read)
		{
			if (!Load(path)) {
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
			// �����дģʽ��������д��汾��
			mCurrentArchiveVersion = BinaryArchive::currentArchiveVersion;
			mDataSize = 128;
			mDataBuffer = new char[mDataSize];

			this->operator<<(mCurrentArchiveVersion);
		}
	}

	void BinaryArchive::SetPath(const char* path)
	{
		Close();
		OpenBinaryFile(path);
		ArchiveBase::SetPath(path);
	}

	void BinaryArchive::WriteImpl(const void* data, U32 size)
	{
		// ����µĴ�С���ڷ����С�������·���2���Ĵ�С
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