#pragma once

#include "resource.h"

namespace Cjing3D
{
	class BaseFileSystem;

	static const char* COMPILED_PATH_NAME = ".assets";
	static const char* COMPILED_PATH_LIST = ".assets/_list.txt";
	static const char* TEMP_COMPILED_PATH_LIST = ".assets/tmp_list.txt";

	namespace ResourceManager
	{
		void Initialize(BaseFileSystem* filesystem);
		void Uninitialize();
		bool IsInitialized();
		void RegisterFactory(ResourceType type, ResourceFactory* factory);
		void UnregisterFactory(ResourceType type);

		Resource* LoadResource(ResourceType type, const Path& inPath, bool isImmediate);

		// TODO; move the part about converter to editor's assertCompiler
		Path GetResourceConvertedPath(const Path& inPath);
		bool CheckResourceNeedConvert(const Path& inPath, Path* outPath = nullptr);
		bool ConvertResource(Resource* resource, ResourceType type, const Path& inPath, bool isImmediate = false);

		void RegisterExtension(const char* ext, ResourceType type);
		ResourceType GetResourceType(const char* ext);

		template<typename T>
		T* LoadResource(const Path& inPath)
		{
			return static_cast<T*>(LoadResource(T::ResType, inPath, false));
		}

		template<typename T>
		T* LoadResourceImmediate(const Path& inPath)
		{
			return static_cast<T*>(LoadResource(T::ResType, inPath, true));
		}

		void ReloadResource(Resource* resource);
		void AcquireResource(Resource* resource);
		bool ReleaseResource(Resource** resource);
		bool IsResourceLoaded(Resource* resource);
		void ProcessReleasedResources();
		void WaitForResource(Resource* resource);
		void WaitAll();

		ResourceType GetResourceType(const char* path);

		enum AsyncResult
		{
			EMPTY = 0,
			PENDING,
			RUNNING,
			SUCCESS,
			FAILURE
		};

		struct AsyncHandle
		{
			volatile I64 mRemaining = 0;
			volatile AsyncResult mResult = AsyncResult::EMPTY;

			AsyncHandle() = default;
			AsyncHandle(const AsyncHandle& rhs) = delete;

			bool IsComplete()const {
				return mResult == AsyncResult::SUCCESS || mResult == AsyncResult::FAILURE;
			}
		};
		AsyncResult ReadFileData(const char* path, char*& buffer, size_t& size, AsyncHandle* asyncHanlde = nullptr);
		AsyncResult WriteFileData(const char* path, void* buffer, size_t size,  AsyncHandle* asyncHanlde = nullptr);

		struct LoadHook
		{
			LoadHook() = default;
			virtual ~LoadHook() {}

			enum HookResult { 
				IMMEDIATE,
				DEFERRED 
			};
			virtual HookResult OoBeforeLoad(Resource* res) = 0;
			virtual void OnWait() = 0;

			void ContinueLoad(Resource* res, bool isImmediate = false);
		};
		void SetCurrentLoadHook(LoadHook* loadHook);
	};
}