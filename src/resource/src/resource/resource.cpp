#include "resource.h"
#include "core\string\stringUtils.h"

namespace  Cjing3D
{
	const ResourceType ResourceType::INVALID_TYPE("");

	ResourceType::ResourceType(const char* type) 
	{
		assert(type[0] == 0 || (type[0] >= 'a' && type[0] <= 'z') || (type[0] >= 'A' && type[0] <= 'Z'));
		mTypeValue = StringUtils::StringToHash(type);
	}

	Resource::Resource() :
		mPath(),
		mRefCount(0),
		mLoaded(0)
	{
	}

	Resource::Resource(const Path& path) :
		mPath(path),
		mRefCount(0),
		mLoaded(0)
	{
	}

	Resource::~Resource()
	{
	}
}