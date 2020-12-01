#pragma once

#include "core\memory\memory.h"
#include "resource.h"

namespace Cjing3D
{
	class BaseResRef
	{
	protected:
		Resource* mResource = nullptr;

	public:
		BaseResRef();
		BaseResRef(Resource* res);
		BaseResRef(const char* path, ResourceType type);
		~BaseResRef();

		BaseResRef(BaseResRef&& rhs);
		BaseResRef& operator=(BaseResRef&& rhs);
		BaseResRef(const BaseResRef& rhs);
		BaseResRef& operator=(const BaseResRef& rhs);

		void Reset();
		bool IsLoaded()const;
		void WaitUntilLoaded();

		explicit operator bool() const { 
			return mResource != nullptr;
		}
	};

	// Resoruce reference
	template<typename T>
	class ResRef : public BaseResRef
	{
	public:
		ResRef() = default;
		ResRef(T* ptr) : BaseResRef(ptr) {}
		ResRef(const char* path) : BaseResRef(path, T::ResType) {}
		
		ResRef(ResRef&& rhs) = default;
		ResRef& operator=(ResRef&& rhs) = default;
		ResRef(const ResRef& rhs) = default;
		ResRef& operator=(const ResRef& rhs) = default;

		T* operator->() { 
			return reinterpret_cast<T*>(mResource);
		}
		const T* operator->()const { 
			return reinterpret_cast<const T*>(mResource);
		}
		operator const T* () const { 
			return reinterpret_cast<const T*>(mResource);
		}
		operator T* () {
			return reinterpret_cast<T*>(mResource);
		}
		T* Ptr() { 
			return reinterpret_cast<T*>(mResource);
		}
		const T* Ptr()const { 
			return reinterpret_cast<const T*>(mResource);
		}
	};
	                                          
}