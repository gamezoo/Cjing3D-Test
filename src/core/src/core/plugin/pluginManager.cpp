#include "pluginManager.h"
#include "core\filesystem\path.h"
#include "core\container\dynamicArray.h"
#include "core\container\hashMap.h"
#include "core\jobsystem\concurrency.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
	//////////////////////////////////////////////////////////////////////////
	// Impl
	//////////////////////////////////////////////////////////////////////////
	class PluginDesc
	{
	private:
		using GetPluginFunc = Plugin * (*)();

		String mPath;
		// String mTempPath;
		void*  mHandle;
		Plugin* mPlugin;
		bool mIsStaticPlugin;

	public:
		PluginDesc(const char* path) :
			mPath(path),
			mHandle(nullptr),
			mPlugin(nullptr),
			mIsStaticPlugin(false)
		{
			// setup temp path
			//MaxPathString fileName, parentPath;
			//Path::GetPathBaseName(mPath, fileName.toSpan());
			//Path::GetPathParentPath(mPath, parentPath.toSpan());

			Reload();
		}

		~PluginDesc()
		{
			if (mHandle) {
				Platform::LibraryClose(mHandle);
			}
			CJING_SAFE_DELETE(mPlugin);
		}

		StringID GetPluginType()const {
			return mPlugin ? mPlugin->GetType() : StringID::EMPTY;
		}

		bool Reload()
		{
			// first clear old data
			if (mHandle) {
				Platform::LibraryClose(mHandle);
			}
			CJING_SAFE_DELETE(mPlugin);

			// try to open lib
			Logger::Info("Loading plugin:%s", mPath.c_str());
			void* handle = Platform::LibraryOpen(mPath.c_str());
			if (handle != nullptr)
			{
				// check for GetPlguin
				GetPluginFunc getPluginFunc = (GetPluginFunc)Platform::LibrarySymbol(handle, "GetPlugin");
				if (!getPluginFunc)
				{
					Debug::Warning("Failed to load plugin libaray:%s", mPath.c_str());
					Platform::LibraryClose(handle);
					return false;
				}

				Plugin* plugin = getPluginFunc();
				if (plugin == nullptr)
				{
					Debug::Warning("Failed to load plugin libaray:%s", mPath.c_str());
					Platform::LibraryClose(handle);
					return false;
				}

				mPlugin = plugin;
				mHandle = handle;
				mIsStaticPlugin = false;

				Logger::Info("Plugin loaded");
				return true;
			}
			else
			{
				// try to load plugin from static plugins
				MaxPathString fileName;
				Path::GetPathBaseName(mPath, fileName.toSpan());
				Plugin* plugin = StaticPlugin::GetPlugin(fileName.c_str());
				if (plugin != nullptr)
				{
					Logger::Info("Plugin loaded");
					mPlugin = plugin;
					mIsStaticPlugin = true;
					return true;
				}
				else
				{
					Debug::Warning("Failed to load plugin libaray:%s", mPath.c_str());
					return false;
				}
			}
		}

		Plugin* GetPlugin() {
			return mPlugin;
		}

		operator bool() const { 
			return mPlugin != nullptr; 
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// Mangaer
	//////////////////////////////////////////////////////////////////////////
	namespace 
	{
		Path GetPluginFullPath(const char* path)
		{
#if CJING3D_PLATFORM_WIN32
			const char* ext = "dll";
#endif
			MaxPathString fullPathStr = path;
			char extTemp[32];
			Path::GetPathExtension(fullPathStr.toSpan(), extTemp);
			if (!EqualString(extTemp, ext))
			{
				fullPathStr.append('.');
				fullPathStr.append(ext);
			}

			return Path(fullPathStr.c_str());
		}
	}

	struct PluginMangaerImpl
	{
		HashMap<U32, PluginDesc*> mPluginDesc;
		Concurrency::Mutex mMutex;
	};
	PluginMangaerImpl* mImpl = nullptr;

	void PluginManager::Initialize()
	{
		Debug::CheckAssertion(mImpl == nullptr);
		mImpl = CJING_NEW(PluginMangaerImpl);
	}

	void PluginManager::Uninitialize()
	{
		Debug::CheckAssertion(mImpl != nullptr);
		for (auto kvp : mImpl->mPluginDesc)
		{
			if (kvp.second != nullptr) {
				CJING_DELETE(kvp.second);
			}
		}
		mImpl->mPluginDesc.clear();

		CJING_SAFE_DELETE(mImpl);
	}

	bool PluginManager::IsInitialized()
	{
		return mImpl != nullptr;
	}

	Plugin* PluginManager::LoadPlugin(const char* path)
	{
		Debug::CheckAssertion(IsInitialized());
		Concurrency::ScopedMutex lock(mImpl->mMutex);

		Path fullPath = GetPluginFullPath(path);
		auto it = mImpl->mPluginDesc.find(fullPath.GetHash());
		if (it != nullptr) 
		{
			PluginDesc* desc = *it;
			if (!(*desc)) {
				desc->Reload();
			}

			if (*desc) {
				return desc->GetPlugin();
			}

			// erase current desc
			mImpl->mPluginDesc.erase(fullPath.GetHash());
			CJING_SAFE_DELETE(desc);
		}

		PluginDesc* desc = CJING_NEW(PluginDesc)(fullPath.c_str());
		if (*desc)
		{
			mImpl->mPluginDesc.insert(fullPath.GetHash(), desc);
			return desc->GetPlugin();
		}
		else
		{
			CJING_SAFE_DELETE(desc);
			return nullptr;
		}
	}

	bool PluginManager::ReloadPlugin(Plugin*& plugin)
	{
		Debug::CheckAssertion(IsInitialized());
		Concurrency::ScopedMutex lock(mImpl->mMutex);

		Path fullPath(plugin->mFileName);
		auto it = mImpl->mPluginDesc.find(fullPath.GetHash());
		if (it == nullptr) {
			return false;
		}

		if ((*it)->Reload())
		{
			plugin = GetPlugin(fullPath.c_str());
			return plugin != nullptr;
		}
		else
		{
			mImpl->mPluginDesc.erase(fullPath.GetHash());
			return false;
		}
	}

	I32 PluginManager::ScanPlugins(const char* rootPath)
	{
		Debug::CheckAssertion(IsInitialized());
		Concurrency::ScopedMutex lock(mImpl->mMutex);

#if CJING3D_PLATFORM_WIN32
		const char* ext = "dll";
#endif
		size_t fileCount = File::EnumerateFiles(rootPath, ext);
		if (fileCount > 0)
		{
			DynamicArray<FileInfo> fileInfos;
			fileInfos.resize(fileCount);

			File::EnumerateFiles(rootPath, ext, fileInfos.data());
			for (int i = 0; i < fileCount; i++)
			{
				const FileInfo& fileInfo = fileInfos[i];

				Path fullpath(fileInfo.mFilename);
				auto it = mImpl->mPluginDesc.find(fullpath.GetHash());
				if (it == nullptr)
				{
					PluginDesc* desc = CJING_NEW(PluginDesc)(fileInfo.mFilename);
					if (*desc) {
						mImpl->mPluginDesc.insert(fullpath.GetHash(), desc);
					}
					else {
						CJING_SAFE_DELETE(desc);
					}
				}
			}
		}

		return mImpl->mPluginDesc.size();
	}

	Plugin* PluginManager::GetPlugin(const char* path)
	{
		Debug::CheckAssertion(IsInitialized());
		Concurrency::ScopedMutex lock(mImpl->mMutex);

		Path fullPath = GetPluginFullPath(path);
		auto it = mImpl->mPluginDesc.find(fullPath.GetHash());
		if (it == nullptr) {
			return nullptr;
		}

		return *it ? (*it)->GetPlugin() : nullptr;
	}

	void PluginManager::GetPlugins(const StringID& pluginType, DynamicArray<Plugin*>& plugins)
	{
		Debug::CheckAssertion(IsInitialized());
		Concurrency::ScopedMutex lock(mImpl->mMutex);

		for (auto kvp : mImpl->mPluginDesc)
		{
			PluginDesc* desc = kvp.second;
			if ((*desc) && desc->GetPluginType() == pluginType) {
				plugins.push(desc->GetPlugin());
			}
		}
	}
}
