#pragma once

#include "resource.h"

namespace Cjing3D
{
	/////////////////////////////////////////////////////////////////////////////////
	// Resource Manager
	namespace ResourceManager
	{
		/// ////////////////////////////////////////////////////////////////////////
		void Initialize();
		void Uninitialize();
		bool IsInitialized();
		void RegisterFactory(ResourceType type, ResourceFactory* factory);
		void UnregisterFactory(ResourceType type);

		/// ////////////////////////////////////////////////////////////////////////
		template<typename T>
		T* LoadResource(const Path& inPath)
		{
			return static_cast<T*>(LoadResource(T::ResType, inPath));
		}

		Resource* LoadResource(ResourceType type, const Path& inPath);
	};
}