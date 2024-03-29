#pragma once

#include "core\common\common.h"
#include "core\filesystem\path.h"
#include "core\filesystem\file.h"
#include "core\concurrency\concurrency.h"
#include "core\container\dynamicArray.h"
#include "core\signal\connectionMap.h"

namespace Cjing3D
{
	/////////////////////////////////////////////////////////////////////////////////
	// Resource
	class ResourceType
	{
	public:
		ResourceType() = default;
		ResourceType(const char* type);

		bool operator==(const ResourceType& rhs) const { 
			return mTypeValue == rhs.mTypeValue; 
		}
		bool operator!=(const ResourceType& rhs) const { 
			return mTypeValue != rhs.mTypeValue;
		}
		bool operator< (const ResourceType& rhs) const {
			return mTypeValue < rhs.mTypeValue;
		}
		U32 Type()const { 
			return mTypeValue; 
		}

		static const ResourceType INVALID_TYPE;

	private:
		U32 mTypeValue = 0;
	}; 

	// Compiled resource header
#pragma pack(1)
	struct CompiledResourceHeader 
	{
		static const U32 MAGIC;
		static const I32 MAJOR = 0;
		static const I32 MINOR = 1;

		U32 mMagic = MAGIC;
		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;

		enum Flags 
		{
			COMPRESSED_LZ4 = 1 << 0
		};
		U32 mFlags = 0;
		U32 mPadding = 0;
		U32 mDecompressedSize = 0;
	};
#pragma pack()

	class Resource
	{
	public:
		enum class ResState
		{
			EMPTY = 0,
			LOADED,
			FAILURE,
		};

		Resource();
		Resource(const Path& path);
		virtual ~Resource();

		void SetDesiredState(ResState state) { mDesiredState = state; }
		ResState GetState()const { return mState; }
		ResState GetDesiredState()const { return mDesiredState; }
		bool IsFaild()const  { return mState == ResState::FAILURE; }
		bool IsLoaded()const { return mState == ResState::LOADED; }
		bool IsEmpty()const  { return mState == ResState::EMPTY; }
		bool IsNeedLoad()const { return IsEmpty() && mDesiredState == ResState::EMPTY; }
		bool WantLoad()const { return mDesiredState == ResState::LOADED; }

		Path GetPath()const { return mPath; }
		void SetPath(const Path& path) { mPath = path; }
		Path GetConvertedPath()const { return mConvertedPath; }
		void SetConvertedPath(const Path& path) { mConvertedPath = path; }
		I32  GetCompiledSize()const { return mCompiledSize; }
		void SetCompiledSize(I32 size) { mCompiledSize = size; }

		virtual ResourceType GetType()const = 0;

	public:
		void SetSourceFiles(const DynamicArray<String>& srcFiles);
		DynamicArray<String> GetSourceFiles()const;
		void AddDependency(Resource& res);
		void RemoveDependency(Resource& res);
		void OnLoaded(bool ret);
		void OnUnloaded();

		Signal<void(ResState oldState, ResState newState)> OnStateChangedSignal;

	public:
		I32 AddRefCount() {
			return Concurrency::AtomicIncrement(&mRefCount);
		}
		I32 SubRefCount() {
			return Concurrency::AtomicDecrement(&mRefCount);
		}
		I32 GetRefCount()const {
			return mRefCount;
		}

	protected:
		volatile I32 mRefCount;
		volatile I32 mEmpty;
		volatile I32 mFailed;

		void CheckState();
		void OnStateChanged(ResState oldState, ResState newState);

	private:
		Path mPath;
		Path mConvertedPath;
		DynamicArray<String> mSourceFiles;
		ResState mState;
		ResState mDesiredState;
		I32 mCompiledSize = 0;

		ConnectionMap mConnectionMap;
	};

	/////////////////////////////////////////////////////////////////////////////////
	// Resource Factory
	class ResourceFactory
	{
	public:
		ResourceFactory() = default;
		virtual~ResourceFactory() {};

		virtual Resource* CreateResource() = 0;
		virtual bool LoadResource(Resource* resource, const char*name, U64 size, const U8* data) = 0;
		virtual bool DestroyResource(Resource* resource) = 0;
		virtual void RegisterExtensions() = 0;

		bool LoadResourceFromFile(Resource* resource, const char* name, U64 size, const U8* data);
	};

#define DECLARE_RESOURCE(CLASS_NAME, NAME)                                                      \
	static const ResourceType ResType;                                                          \
	virtual ResourceType GetType()const { return ResType; };                                    \
	static void RegisterFactory();                                                              \
	static void UnregisterFactory();                                                            \
	static class CLASS_NAME##Factory* GetFactory();                                             \
	friend class CLASS_NAME##Factory;

#define DEFINE_RESOURCE(CLASS_NAME, NAME)                                                       \
	const ResourceType CLASS_NAME::ResType(NAME);                                               \
	static CLASS_NAME##Factory* mFactory = nullptr;                                             \
	void CLASS_NAME::RegisterFactory()                                                          \
	{                                                                                           \
		assert(mFactory == nullptr);                                                            \
		mFactory = CJING_NEW(CLASS_NAME##Factory)();                                            \
		ResourceManager::RegisterFactory(CLASS_NAME::ResType, mFactory);                        \
	}                                                                                           \
	void CLASS_NAME::UnregisterFactory()                                                        \
	{                                                                                           \
		assert(mFactory != nullptr);                                                            \
		ResourceManager::UnregisterFactory(CLASS_NAME::ResType);                                \
		CJING_SAFE_DELETE(mFactory);															\
	}                                                                                           \
	class CLASS_NAME##Factory* CLASS_NAME::GetFactory()                                         \
	{                                                                                           \
		assert(mFactory != nullptr);                                                            \
		return mFactory;                                                                        \
	}                                                                                                                  
}