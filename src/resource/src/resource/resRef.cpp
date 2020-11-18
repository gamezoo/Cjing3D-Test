#include "resRef.h"
#include "resourceManager.h"

namespace Cjing3D
{
	BaseResRef::BaseResRef()
	{
	}

	BaseResRef::BaseResRef(Resource* res) :
		mResource(res)
	{
	}

	BaseResRef::BaseResRef(const char* path, ResourceType type)
	{
		mResource = ResourceManager::LoadResource(type, Path(path), true);
	}

	BaseResRef::~BaseResRef()
	{
		Reset();
	}

	BaseResRef::BaseResRef(BaseResRef&& rhs)
	{
		std::swap(mResource, rhs.mResource);
	}

	BaseResRef& BaseResRef::operator=(BaseResRef&& rhs)
	{
		std::swap(mResource, rhs.mResource);
		return *this;
	}

	BaseResRef::BaseResRef(const BaseResRef& rhs)
	{
		if (rhs.mResource != nullptr)
		{
			mResource = rhs.mResource;
			ResourceManager::AcquireResource(mResource);
		}
	}

	BaseResRef& BaseResRef::operator=(const BaseResRef& rhs)
	{
		Reset();

		if (rhs.mResource != nullptr)
		{
			mResource = rhs.mResource;
			ResourceManager::AcquireResource(mResource);
		}
		return *this;
	}

	void BaseResRef::Reset()
	{
		if (mResource != nullptr) 
		{
			ResourceManager::ReleaseResource(&mResource);
			mResource = nullptr;
		}
	}

	bool BaseResRef::IsLoaded() const
	{
		return ResourceManager::IsResourceLoaded(mResource);
	}

	void BaseResRef::WaitUntilLoaded()
	{
		if (mResource != nullptr) {
			ResourceManager::WaitForResource(mResource);
		}
	}
}