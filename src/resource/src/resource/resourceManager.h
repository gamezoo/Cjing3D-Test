#pragma once

#include "resource.h"

namespace Cjing3D
{
	class BaseFileSystem;

	static const char* COMPILED_PATH_NAME = "converter_output";
	static const char* COMPILED_PATH_LIST = "converter_output/assetList.txt";

	namespace ResourceManager
	{
		void Initialize(BaseFileSystem* filesystem);
		void Uninitialize();
		bool IsInitialized();
		void RegisterFactory(ResourceType type, ResourceFactory* factory);
		void UnregisterFactory(ResourceType type);

		Resource* LoadResource(ResourceType type, const Path& inPath, bool isImmediate);
		Resource* ConvertResource(ResourceType type, const Path& inPath, bool continueLoad = true);

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

			enum class HookResult { 
				IMMEDIATE,
				DEFERRED 
			};
			virtual HookResult OoBeforeLoad(Resource* res) = 0;
		};
		void SetCurrentLoadHook(LoadHook* loadHook);
	};
}