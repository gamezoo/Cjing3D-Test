#include "factory.h"

namespace Cjing3D
{
	ResourceFactory::ResourceFactory(ResourceType type) :
		mResourceType(type)
	{
	}

	ResourceFactory::~ResourceFactory()
	{
		Clear();
	}

	void ResourceFactory::Clear()
	{
	}

	void ResourceFactory::LoadResource(Resource& resource)
	{
	}

	Resource* ResourceFactory::LoadResource(const Path& path)
	{
		return nullptr;
	}

	void ResourceFactory::UnloadResource(Resource& resource)
	{
	}
}