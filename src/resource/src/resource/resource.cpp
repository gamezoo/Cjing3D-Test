#include "resource.h"
#include "core\string\stringUtils.h"
#include "core\helper\debug.h"
#include "core\helper\stream.h"
#include "core\concurrency\concurrency.h"
#include "core\compress\lz4.h"

namespace  Cjing3D
{
	const U32 CompiledResourceHeader::MAGIC = 0x129A1119;
	const ResourceType ResourceType::INVALID_TYPE("");

	ResourceType::ResourceType(const char* type) 
	{
		assert(type[0] == 0 || (type[0] >= 'a' && type[0] <= 'z') || (type[0] >= 'A' && type[0] <= 'Z'));
		mTypeValue = StringUtils::StringToHash(type);
	}

	Resource::Resource() :
		mPath(),
		mRefCount(0),
		mEmpty(1),
		mFailed(0),
		mState(ResState::EMPTY),
		mDesiredState(ResState::EMPTY)
	{
	}

	Resource::Resource(const Path& path) :
		mPath(path),
		mRefCount(0),
		mEmpty(1),
		mFailed(0),
		mState(ResState::EMPTY),
		mDesiredState(ResState::EMPTY)
	{
	}

	Resource::~Resource()
	{
	}

	void Resource::SetSourceFiles(const DynamicArray<String>& srcFiles)
	{
		mSourceFiles = srcFiles;
	}

	DynamicArray<String> Resource::GetSourceFiles() const
	{
		return mSourceFiles;
	}

	void Resource::AddDependency(Resource& res)
	{
		// bind onStateChanged signal
		mConnectionMap.Connect(
			StringID(res.GetPath().GetHash()),
			res.OnStateChangedSignal, 
			[this](ResState oldState, ResState newState) {
				OnStateChanged(oldState, newState);
			}
		);

		if (res.IsEmpty()) {
			Concurrency::AtomicIncrement(&mEmpty);
		}
		if (res.IsFaild()) {
			Concurrency::AtomicIncrement(&mFailed);
		}
		CheckState();
	}

	void Resource::RemoveDependency(Resource& res)
	{
		mConnectionMap.Disconnect(StringID(res.GetPath().GetHash()));

		if (res.IsEmpty()) 
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
		}
		if (res.IsFaild()) 
		{
			Debug::CheckAssertion(mFailed > 0);
			Concurrency::AtomicDecrement(&mFailed);
		}

		CheckState();
	}

	void Resource::OnLoaded(bool ret)
	{
		if (ret)
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
		}
		else
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
			Concurrency::AtomicIncrement(&mFailed);
		}
		CheckState();
	}

	void Resource::OnUnloaded()
	{
		mDesiredState = ResState::EMPTY;
		mEmpty = 1;
		mFailed = 0;
		CheckState();
	}

	void Resource::CheckState()
	{
		// check state by failed dep and emtpy dep
		ResState oldState = mState;
		if (mState != ResState::FAILURE && mFailed > 0)
		{
			mState = ResState::FAILURE;
			OnStateChangedSignal(oldState, mState);
		}
		else if (mFailed <= 0)
		{
			if (mEmpty == 0 && 
				mState != ResState::LOADED && 
				mDesiredState != ResState::EMPTY)
			{
				mState = ResState::LOADED;
				OnStateChangedSignal(oldState, mState);
			}

			if (mEmpty > 0 && mState != ResState::EMPTY)
			{
				mState = ResState::EMPTY;
				OnStateChangedSignal(oldState, mState);
			}
		}
	}

	void Resource::OnStateChanged(ResState oldState, ResState newState)
	{
		Debug::CheckAssertion(oldState != newState);
		Debug::CheckAssertion(mState != ResState::EMPTY || mDesiredState != ResState::EMPTY);

		// process old states
		if (oldState == ResState::EMPTY)
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
		}
		if (oldState == ResState::FAILURE)
		{
			Debug::CheckAssertion(mFailed > 0);
			Concurrency::AtomicDecrement(&mFailed);
		}
		// process new states
		if (newState == ResState::EMPTY) {
			Concurrency::AtomicIncrement(&mEmpty);
		}
		if (newState == ResState::FAILURE) {
			Concurrency::AtomicIncrement(&mFailed);
		}

		CheckState();
	}

	bool ResourceFactory::LoadResourceFromFile(Resource* resource, const char* name, U64 size, const U8* data)
	{
		// read shader general header
		const CompiledResourceHeader& header = *(const CompiledResourceHeader*)data;

		// check magic and version
		if (header.mMagic != CompiledResourceHeader::MAGIC ||
			header.mMajor != CompiledResourceHeader::MAJOR ||
			header.mMinor != CompiledResourceHeader::MINOR)
		{
			Logger::Warning("resource version mismatch.");
			return false;
		}

		if (header.mFlags & CompiledResourceHeader::COMPRESSED_LZ4)
		{
			// if res is compressed, decompress first
			MemoryStream tmp;
			tmp.Resize(header.mDecompressedSize);
			const I32 res = LZ4_decompress_safe((const char*)data + sizeof(header), (char*)tmp.data(), I32(size - sizeof(header)), (I32)tmp.Size());
			if (res != header.mDecompressedSize) 
			{
				Logger::Warning("resource decompress failed.");
				return false;
			}
			return LoadResource(resource, name, header.mDecompressedSize, tmp.data());
		}
		else
		{
			return LoadResource(resource, name, size - sizeof(header), data + sizeof(header));
		}
	}
}