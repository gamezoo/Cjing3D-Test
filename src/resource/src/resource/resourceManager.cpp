#include "resourceManager.h"
#include "core\helper\debug.h"
#include "core\filesystem\filesystem.h"
#include "core\container\hashMap.h"
#include "core\jobsystem\taskJob.h"

namespace Cjing3D
{
namespace ResourceManager
{
	using FactoryTable = HashMap<U32, ResourceFactory*>;
	//////////////////////////////////////////////////////////////////////////
	// Impl
	//////////////////////////////////////////////////////////////////////////


	class ResourceManagerImpl
	{
	public:
		FactoryTable mResourceFactoires;
		MaxPathString mRootPath;

	public:
		ResourceManagerImpl();
		~ResourceManagerImpl();

		ResourceFactory* GetFactory(ResourceType type);
		Resource* AcquireResource(const Path& path, ResourceType type);
	};

	ResourceManagerImpl::ResourceManagerImpl()
	{
	}

	ResourceManagerImpl::~ResourceManagerImpl()
	{
	}

	ResourceFactory* ResourceManagerImpl::GetFactory(ResourceType type)
	{
		auto it = mResourceFactoires.find(type.Type());
		return it != nullptr ? *it : nullptr;
	}

	Resource* ResourceManagerImpl::AcquireResource(const Path& path, ResourceType type)
	{
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	// Manager
	//////////////////////////////////////////////////////////////////////////
	ResourceManagerImpl* mImpl = nullptr;

	void Initialize()
	{
		Debug::CheckAssertion(mImpl == nullptr);
		mImpl = CJING_NEW(ResourceManagerImpl);
	}

	void Uninitialize()
	{
		Debug::CheckAssertion(mImpl != nullptr);
		CJING_SAFE_DELETE(mImpl);
	}

	bool IsInitialized()
	{
		return mImpl != nullptr;
	}

	void RegisterFactory(ResourceType type, ResourceFactory* factory)
	{
		if (!IsInitialized()) {
			return;
		}
		mImpl->mResourceFactoires.insert(type.Type(), factory);
	}

	void UnregisterFactory(ResourceType type)
	{
		if (!IsInitialized()) {
			return;
		}
		mImpl->mResourceFactoires.erase(type.Type());
	}

	Resource* LoadResource(ResourceType type, const Path& inPath)
	{
		Debug::CheckAssertion(IsInitialized());
		Debug::CheckAssertion(!inPath.IsEmpty());

		MaxPathString path, fileName, ext;
		if (!inPath.SplitPath(path.data(), path.size(), fileName.data(), fileName.size(), ext.data(), ext.size()))
		{
			Debug::Warning("Invalid path:%s", inPath.c_str());
			return nullptr;
		}

		ResourceFactory* factory = mImpl->GetFactory(type);
		if (factory == nullptr)
		{
			Debug::Warning("Invalid res type:%d", type.Type());
			return nullptr;
		}

		// set converted path
		MaxPathString convertedPath, convertedFileName;
		sprintf_s(convertedFileName.data(), convertedFileName.size(), "%s.%s.converted", fileName.data(), ext.data());
		sprintf_s(convertedPath.data(), convertedPath.size(), "%s.converter_output", mImpl->mRootPath.data());

		// acquire resource
		Resource* ret = mImpl->AcquireResource(Path(convertedPath), type);
		if (ret == nullptr) 
		{
			ret = factory->CreateResource();
			if (ret == nullptr) {
				return nullptr;
			}


		}

		return ret;
	}
}
}