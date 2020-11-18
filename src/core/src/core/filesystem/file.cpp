#include "file.h"
#include "core\platform\platform.h"
#include "core\memory\memory.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
	class MemFile : public FileImpl
	{
	private:
		void* mData = nullptr;
		size_t mSize = 0;
		size_t mPos = 0;
		FileFlags mFlags = FileFlags::NONE;

	public:
		MemFile(void* data, size_t size, FileFlags flags) :
			mFlags(flags),
			mData(data),
			mSize(size)
		{
		}

		~MemFile()
		{
		}

		bool Read(void* buffer, size_t bytes)override
		{
			if (FLAG_ANY(mFlags, FileFlags::READ))
			{
				const size_t copySize = std::min(mSize - mPos, bytes);
				Memory::Memcpy(buffer, (const U8*)mData + mPos, copySize);
				mPos += copySize;
				return copySize > 0;
			}
			return false;
		}

		bool Write(const void* buffer, size_t bytes)override
		{
			if (FLAG_ANY(mFlags, FileFlags::WRITE))
			{
				const size_t copySize = std::min(mSize - mPos, bytes);
				Memory::Memcpy((U8*)mData + mPos, buffer, copySize);
				mPos += copySize;
				return copySize > 0;
			}
			return false;
		}

		bool Seek(size_t offset)override
		{
			if (offset < mSize)
			{
				mPos = offset;
				return true;
			}
			return false;
		}

		size_t Tell() const override
		{
			return mPos;
		}

		size_t Size() const override {
			return mSize;
		}

		FileFlags GetFlags() const override {
			return mFlags;
		}

		bool IsValid() const override {
			return mData != nullptr;
		}

		const char* GetPath() const override {
			return "";
		}
	};

#ifdef CJING3D_PLATFORM_WIN32
	class FileWin32 : public FileImpl
	{
	private:
		size_t mSize = 0;
		FileFlags mFlags = FileFlags::NONE;
		HANDLE mHandle = INVALID_HANDLE_VALUE;
		volatile int mMappedCount = 0;
#ifdef DEBUG
		String mPath;
#endif

	public:
		FileWin32(const char* path, FileFlags flags, FilePathResolver* resolver) :
			mFlags(flags)
		{
			if (resolver != nullptr) {
				// doing something
			}

			// set flags
			DWORD desiredAccess = 0;
			DWORD shareMode = 0;
			DWORD createFlags = 0;
			DWORD fileFlags = 0;

			if (FLAG_ANY(flags, FileFlags::CREATE))
			{
				if (Platform::FileExists(path)) {
					createFlags = TRUNCATE_EXISTING;
				}
				else {
					createFlags = CREATE_ALWAYS;
				}
			}
			if (FLAG_ANY(flags, FileFlags::READ))
			{
				desiredAccess |= GENERIC_READ;
				shareMode = FILE_SHARE_READ;
			}
			if (FLAG_ANY(flags, FileFlags::WRITE))
			{
				desiredAccess |= GENERIC_WRITE;
			}

			mHandle = ::CreateFileA(path, desiredAccess, shareMode, nullptr, createFlags, fileFlags, 0);
			if (mHandle == INVALID_HANDLE_VALUE)
			{
				Debug::Error("Failed to create file:\"%s\", error:%x", path, ::GetLastError());
			}
			else
			{
				DWORD sizeH = 0;
				DWORD sizeL = ::GetFileSize(mHandle, &sizeL);
				mSize = (size_t)(sizeH) << 32ull | sizeL;
			}

#ifdef DEBUG
			mPath = path;
#endif
		}

		~FileWin32()
		{
			if (mHandle != INVALID_HANDLE_VALUE)
			{
				::FlushFileBuffers(mHandle);
				::CloseHandle(mHandle);
			}
		}

		bool Read(void* buffer, size_t bytes)override
		{
			U8* readBuffer = static_cast<U8*>(buffer);
			DWORD readed = 0;
			BOOL success = ::ReadFile(mHandle, readBuffer, (DWORD)bytes, (LPDWORD)&readed, nullptr);
			return success && bytes == readed;
		}

		bool Write(const void* buffer, size_t bytes)override
		{
			size_t written = 0;
			const U8* writeBuffer = static_cast<const U8*>(buffer);
			BOOL success = ::WriteFile(mHandle, writeBuffer, (DWORD)bytes, (LPDWORD)&written, nullptr);
			return success && bytes == written;
		}

		bool Seek(size_t offset)override
		{
			const LONG offsetHi = offset >> 32u;
			const LONG offsetLo = offset & 0xffffffffu;
			LONG offsetHiOut = offsetHi;
			return ::SetFilePointer(mHandle, offsetLo, &offsetHiOut, FILE_BEGIN) == (DWORD)offsetLo && offsetHiOut == offsetHi;
		}

		size_t Tell() const override
		{
			LONG offsetHi = 0;
			DWORD offsetLo = ::SetFilePointer(mHandle, 0, &offsetHi, FILE_CURRENT);
			return (size_t)offsetHi << 32ull | offsetLo;
		}

		size_t Size() const override {
			return mSize;
		}

		FileFlags GetFlags() const override {
			return mFlags;
		}

		bool IsValid() const override {
			return mHandle != INVALID_HANDLE_VALUE;
		}

		const char* GetPath() const override {
#ifdef DEBUG
			return mPath.c_str();
#else
			return "";
#endif
		}
	};
#endif

	File::File(const char* path, FileFlags flags, FilePathResolver* resolver)
	{
#ifdef CJING3D_PLATFORM_WIN32
		mFileImpl = CJING_NEW(FileWin32)(path, flags, resolver);
#endif
	}

	File::File(void* data, size_t size, FileFlags flags)
	{
		mFileImpl = CJING_NEW(MemFile)(data, size, flags);
	}

	File::File(File&& rhs)noexcept
	{
		std::swap(mFileImpl, rhs.mFileImpl);
	}

	File::File(FileImpl* fileImpl) :
		mFileImpl(fileImpl)
	{
	}

	File& File::operator=(File&& rhs)noexcept
	{
		std::swap(mFileImpl, rhs.mFileImpl);
		return *this;
	}

	File::~File()
	{
		CJING_SAFE_DELETE(mFileImpl);
	}

	size_t File::Read(void* buffer, size_t bytes)
	{
		Debug::CheckAssertion(FLAG_ANY(GetFlags(), FileFlags::READ));
		return mFileImpl->Read(buffer, bytes);
	}

	size_t File::Write(const void* buffer, size_t bytes)
	{
		Debug::CheckAssertion(FLAG_ANY(GetFlags(), FileFlags::WRITE));
		return mFileImpl->Write(buffer, bytes);
	}

	bool File::Seek(size_t offset)
	{
		return mFileImpl->Seek(offset);
	}

	size_t File::Tell() const
	{
		return mFileImpl->Tell();
	}

	size_t File::Size() const
	{
		return mFileImpl->Size();
	}

	FileFlags File::GetFlags() const
	{
		return mFileImpl->GetFlags();
	}

	const char* File::GetPath() const
	{
		return mFileImpl->GetPath();
	}

	bool File::IsValid() const
	{
		return mFileImpl->IsValid();
	}
}
