#pragma once

#include "resource.h"

namespace Cjing3D
{
	namespace ResourceManager
	{
		void Initialize(const char* rootPath);
		void Uninitialize();
		bool IsInitialized();
		void RegisterFactory(ResourceType type, ResourceFactory* factory);
		void UnregisterFactory(ResourceType type);

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
		void WaitForResource(Resource* resource);

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