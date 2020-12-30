#pragma once

#include "resource.h"

namespace Cjing3D
{
	class BaseFileSystem;

	namespace ResourceManager
	{
		void Initialize(BaseFileSystem* filesystem);
		void Uninitialize();
		bool IsInitialized();
		void RegisterFactory(ResourceType type, ResourceFactory* factory);
		void UnregisterFactory(ResourceType type);
		void SetConvertEnable(bool convertEnable);

		Resource* LoadResource(ResourceType type, const Path& inPath, bool isImmediate);

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
	};
}