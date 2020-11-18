#pragma once

#include "core\common\common.h"
#include "core\filesystem\path.h"
#include "core\filesystem\file.h"
#include "core\jobsystem\concurrency.h"

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

	class Resource
	{
	public:
		Resource();
		Resource(const Path& path);
		virtual ~Resource();

		bool IsLoaded()const { return mLoaded != 0; }
		Path GetPath()const { return mPath; }
		void SetPath(const Path& path) { mPath = path; }

		virtual ResourceType GetType()const = 0;

	public:
		void AddRefCount() {
			Concurrency::AtomicIncrement(&mRefCount);
		}
		void SubRefCount() {
			Concurrency::AtomicDecrement(&mRefCount);
		}
		I32 GetRefCount()const {
			return mRefCount;
		}
		volatile I32 mRefCount;
		volatile I32 mLoaded;

	protected:
		Path mPath;
	};

	/////////////////////////////////////////////////////////////////////////////////
	// Resource Factory
	class ResourceFactory
	{
	public:
		ResourceFactory() = default;
		virtual~ResourceFactory() {};

		virtual Resource* CreateResource() = 0;
		virtual bool LoadResource(Resource* resource, const char*name, File& file) = 0;
		virtual bool DestroyResource(Resource* resource) = 0;
		virtual bool IsNeedConvert()const = 0;
	};

#define DECLARE_RESOURCE(CLASS_NAME, NAME)                                                      \
	static const ResourceType ResType;                                                             \
	virtual ResourceType GetType()const { return ResType; };                                       \
	static void RegisterFactory();                                                              \
	static void UnregisterFactory();                                                            \
	static class CLASS_NAME##Factory* GetFactory();

#define DEFINE_RESOURCE(CLASS_NAME, NAME)                                                       \
	const ResourceType CLASS_NAME::ResType(NAME);                                                 \
	static CLASS_NAME##Factory* mFactory = nullptr;                                             \
	void CLASS_NAME::RegisterFactory()                                                          \
	{                                                                                           \
		assert(mFactory == nullptr);                                                            \
		mFactory = CJING_NEW(CLASS_NAME##Factory)();                                            \
		ResourceManager::RegisterFactory(CLASS_NAME::ResType, mFactory);                           \
	}                                                                                           \
	void CLASS_NAME::UnregisterFactory()                                                        \
	{                                                                                           \
		assert(mFactory != nullptr);                                                            \
		ResourceManager::UnregisterFactory(CLASS_NAME::ResType);                                   \
		CJING_SAFE_DELETE(mFactory);															\
	}                                                                                           \
	class CLASS_NAME##Factory* CLASS_NAME::GetFactory()                                         \
	{                                                                                           \
		assert(mFactory != nullptr);                                                            \
		return mFactory;                                                                        \
	}                                                                                                                  
}