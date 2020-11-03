#pragma once

#include <type_traits>
#include <utility>
#include <map>
#include <vector>
#include <memory>
#include <cassert>

namespace Cjing3D
{
namespace Reflection
{
	class  Handle;
	class  Any;
	class  Type;
	class  Variable;

	template<typename T>
	using RemoveCVR = std::remove_cv_t<std::remove_reference_t<T>>;

	///////////////////////////////////////////////////////////////////////////////////////
	// UID
	///////////////////////////////////////////////////////////////////////////////////////
	using UID = size_t;
	static UID DEFAULT_UID = 0;

	inline unsigned int SDBMHash(unsigned int hash, unsigned char c)
	{
		return c + (hash << 6) + (hash << 16) - hash;
	}
	UID CalculateHash(const char* str)
	{
		if (str == nullptr) {
			return 0;
		}

		UID hashValue = 0;
		while (*str != 0) {
			hashValue = SDBMHash(hashValue, (unsigned char)*str++);
		}
		return hashValue;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Impl
	///////////////////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		struct TypeInfo;

		///////////////////////////////////////////////////////////////////////////////
		// Info definition
		///////////////////////////////////////////////////////////////////////////////
		struct CtorInfo
		{

		};

		struct DtorInfo
		{

		};

		struct VarInfo
		{
			UID mIdentifier;
			const TypeInfo* mParent;
			const bool mIsConst;
			const bool mIsStatic;
			bool (* const Set)(Handle, size_t, Any);
			Any  (* const Get)(Handle, size_t);
			Variable(* const ToVar)() noexcept;
			TypeInfo* (* const GetTypeInfo)()noexcept;
		};

		struct FuncInfo
		{
			UID mIdentifier;
		};

		struct TypeInfo
		{
			UID mIdentifier;
			const bool mIsClass;
			const bool mIsArray;
			const size_t mExtent;
			TypeInfo* mParent = nullptr;
			std::vector<CtorInfo*> mCtors;
			DtorInfo* mDtor = nullptr;
			Type(* const ToType)() noexcept;
			std::map<UID, VarInfo*>  mVars;
			std::map<UID, FuncInfo*> mFuncs;
		};

		template<typename T>
		struct InfoFactory
		{
			inline static TypeInfo* mTypeInfo = nullptr;
			inline static TypeInfo* Resolve()noexcept;
		};

		template<typename T>
		const T* TryCast(const TypeInfo* typeInfo, void* ptr) noexcept
		{
			const auto* tInfo = InfoFactory<T>::Resolve();
			if (tInfo == typeInfo)
			{
				return static_cast<const T*>(ptr);
			}
			else
			{
				// check 
				return nullptr;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Any
	///////////////////////////////////////////////////////////////////////////////
	class Any
	{
	public:
		using StorageType = std::aligned_storage_t<sizeof(void*), alignof(void*)>;
		using DestroyFuncT = void(void*);
		using CopyFuncT = void* (StorageType&, const void*);
		using MoveFuncT = void* (StorageType&, void*, DestroyFuncT*);

		// 如果T类型大小大于指针大小，则创建指针，并在storage存储指针的地址
		template<typename T, typename = std::void_t<>>
		struct type_traits
		{
			template<typename... Args>
			static void* Create(StorageType& storage, Args&&... args)
			{
				auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
				new(&storage) T* { ptr.get() };
				return ptr.release();
			}

			static void Destroy(void* ptr)
			{
				auto* obj = static_cast<T*>(ptr);
				delete obj;
			}

			static void* Copy(StorageType& storage, const void* other)
			{
				auto ptr = std::make_unique<T>(*static_cast<const T*>(other));
				new(&storage) T* { ptr.get() };
				return ptr.release();
			}

			static void* Move(StorageType& storage, void* other, DestroyFuncT* destroyFunc)
			{
				// 对于指针Move：只需要在Storage中记录other地址即可
				auto* newPtr = static_cast<T*>(other);
				new (&storage) T* { newPtr };
				return newPtr;
			};
		};

		// 如果T本身大小小于指针大小（例如基础类型），则直接基于Storage直接创建对象
		template<typename T>
		struct type_traits<T, std::enable_if_t<sizeof(T) <= sizeof(void*) && std::is_nothrow_move_constructible_v<T>>>
		{
			template<typename... Args>
			static void* Create(StorageType& storage, Args&&... args)
			{
				return new(&storage) T{ std::forward<Args>(args)... };
			}

			static void Destroy(void* ptr)
			{
				auto* obj = static_cast<T*>(ptr);
				obj->~T();
			}

			static void* Copy(StorageType& storage, const void* other)
			{
				return new(&storage) T{ *static_cast<const T*>(other) };
			}

			static void* Move(StorageType& storage, void* other, DestroyFuncT* destroyFunc)
			{
				void* newPtr = new(&storage) T{ std::move(*static_cast<const T*>(other)) };
				if (destroyFunc) {
					destroyFunc(other);
				}
				return newPtr;
			};
		};

	public:
		Any() :
			mStorageType{},
			mTypeInfo(nullptr),
			mInstance(nullptr)
		{}

		// in_place_type来明确表示要以哪个类型初始化，然后args是这个类型的构造参数
		template<typename T, typename... Args>
		Any(std::in_place_type_t<T>, Args&&... args) : Any()
		{
			mTypeInfo = Impl::InfoFactory<T>::Resolve();

			if constexpr (!std::is_void_v<T>)
			{
				using traits_type = type_traits<std::remove_cv_t<std::remove_reference_t<T>>>;
				mInstance = traits_type::Create(mStorageType, std::forward<Args>(args)...);

				mDestroyFunc = &traits_type::Destroy;
				mCopyFunc = &traits_type::Copy;
				mMoveFunc = &traits_type::Move;
			}
		}

		inline Any(Handle instance) noexcept;

		template<typename T, typename = std::enable_if_t<!std::is_same_v<RemoveCVR<T>, Any>>>
		Any(T&& value) : Any(std::in_place_type<RemoveCVR<T>>, std::forward<T>(value)) {}

		Any(const Any& rhs) : Any()
		{
			mTypeInfo = rhs.mTypeInfo;
			mInstance = rhs.mCopyFunc ? rhs.mCopyFunc(mStorageType, rhs.mInstance) : rhs.mInstance;
			mDestroyFunc = rhs.mDestroyFunc;
			mCopyFunc = rhs.mCopyFunc;
			mMoveFunc = rhs.mMoveFunc;
		}

		Any(Any&& rhs) noexcept
		{
			Swap(*this, rhs);
		}

		~Any()
		{
			if (mInstance && mDestroyFunc) {
				mDestroyFunc(mInstance);
			}
		}

		Any& operator=(const Any& rhs)
		{
			return (*this = Any(rhs));
		}

		Any& operator=(Any&& rhs) noexcept
		{
			Any any{ std::move(rhs) };
			Swap(any, *this);
			return *this;
		}

		template<typename T, typename = std::enable_if_t<std::is_same_v<RemoveCVR<T>, Any>>>
		Any& operator=(T&& value)
		{
			return (*this = Any(std::forward<T>(value)));
		}

		void* Instance() noexcept { return mInstance; }
		const void* Instance()const noexcept { return mInstance; }
		const Impl::TypeInfo* Type()const noexcept { return mTypeInfo; }

		template<typename T>
		const T* TryCast()const noexcept
		{
			return Impl::TryCast<T>(mTypeInfo, mInstance);
		}

		template<typename T>
		T* TryCast() noexcept
		{
			return const_cast<T*>(std::as_const(*this).TryCast<T>());
		}

		void Swap(Any& lhs, Any& rhs) noexcept
		{
			if (lhs.mMoveFunc && rhs.mMoveFunc)
			{
				StorageType tempBuffer;
				auto* temp = rhs.mMoveFunc(tempBuffer, rhs.mInstance, rhs.mDestroyFunc);
				rhs.mInstance = lhs.mMoveFunc(rhs.mStorageType, lhs.mInstance, lhs.mDestroyFunc);
				lhs.mInstance = rhs.mMoveFunc(lhs.mStorageType, temp, rhs.mDestroyFunc);
			}
			else if (lhs.mMoveFunc)
			{
				lhs.mInstance = lhs.mMoveFunc(rhs.mStorageType, lhs.mInstance, lhs.mDestroyFunc);
				std::swap(lhs.mInstance, rhs.mInstance);
			}
			else if (rhs.mMoveFunc)
			{
				rhs.mInstance = rhs.mMoveFunc(lhs.mStorageType, rhs.mInstance, rhs.mDestroyFunc);
				std::swap(lhs.mInstance, rhs.mInstance);
			}

			std::swap(lhs.mTypeInfo, rhs.mTypeInfo);
			std::swap(lhs.mDestroyFunc, rhs.mDestroyFunc);
			std::swap(lhs.mCopyFunc, rhs.mCopyFunc);
			std::swap(lhs.mMoveFunc, rhs.mMoveFunc);
		}

		bool operator==(const Any&& rhs) const noexcept
		{
			return mTypeInfo == rhs.mTypeInfo;
		}

		explicit operator bool() const noexcept
		{
			return mTypeInfo;
		}

		// convert
		template <typename T>
		Any Convert()const
		{
			return Any();
		}

	private:
		const Impl::TypeInfo* mTypeInfo;
		void* mInstance;
		StorageType mStorageType;

		// 根据T类型对应的行为函数
		DestroyFuncT* mDestroyFunc = nullptr;
		CopyFuncT*    mCopyFunc = nullptr;
		MoveFuncT*    mMoveFunc = nullptr;
	};

	///////////////////////////////////////////////////////////////////////////////
	// Instance
	///////////////////////////////////////////////////////////////////////////////
	class Handle
	{
	public:
		Handle() noexcept :
			mTypeInfo(nullptr),
			mInstance(nullptr)
		{}

		template<typename T, typename = std::enable_if_t<!std::is_same_v<RemoveCVR<T>, Handle>>>
		Handle(T& obj) noexcept :
			mTypeInfo(Impl::InfoFactory<T>::Resolve()),
			mInstance(&obj)
		{}

		Handle(Any& any) noexcept :
			mTypeInfo(any.Type()),
			mInstance(any.Instance())
		{
		}

		explicit operator bool() const noexcept
		{
			return mTypeInfo;
		}

		const Impl::TypeInfo* TypeInfo() const noexcept { return mTypeInfo; }
		void* Instance()noexcept { return mInstance; }
		const void* Instance()const noexcept { return mInstance; }
	private:
		const Impl::TypeInfo* mTypeInfo;
		void* mInstance;
	};

	inline Any::Any(Handle instance) noexcept : Any{}
	{
		mTypeInfo = instance.TypeInfo();
		mInstance = instance.Instance();
	}


	///////////////////////////////////////////////////////////////////////////////
	// Variable
	///////////////////////////////////////////////////////////////////////////////
	// impl::VarInfo wrapper
	class Variable
	{
	public:
		Variable() noexcept {}
		Variable(const Impl::VarInfo* varInfo) :
			mVarInfo(varInfo)
		{}

		bool IsConst()const { return mVarInfo->mIsConst; }
		bool IsStatic()const { return mVarInfo->mIsStatic; }

		template<typename T>
		bool Set(Handle handle, T&& value)
		{
			return mVarInfo->Set(handle, 0, Any(std::forward<T>(value)));
		}

		template<typename T>
		bool Set(Handle handle, size_t index, T&& value)
		{
			assert(index < mVarInfo->GetTypeInfo()->mExtent);
			return mVarInfo->Set(handle, index, Any(std::forward<T>(value)));
		}

		Any Get(Handle handle)
		{
			return mVarInfo->Get(handle, 0);
		}

		Any Get(Handle handle, size_t index)
		{
			assert(index < mVarInfo->GetTypeInfo()->mExtent);
			return mVarInfo->Get(handle, index);
		}

		explicit operator bool() const noexcept {
			return mVarInfo;
		}

		bool operator==(const Variable& rhs) const noexcept {
			return mVarInfo == rhs.mVarInfo;
		}

	private:
		const Impl::VarInfo* mVarInfo = nullptr;
	};

	///////////////////////////////////////////////////////////////////////////////
	// Type
	///////////////////////////////////////////////////////////////////////////////
	// impl::TypeInfo wrapper
	class Type
	{
	public:
		Type() noexcept {}
		Type(const Impl::TypeInfo* typeInfo) :
			mTypeInfo(typeInfo)
		{}

		bool IsClass()const {
			return mTypeInfo->mIsClass;
		}

		Variable Var(UID uid)const
		{
			if (!mTypeInfo) {
				return Variable();
			}

			auto it = mTypeInfo->mVars.find(uid);
			return it != mTypeInfo->mVars.end() ? it->second->ToVar() : Variable();
		}

		explicit operator bool() const noexcept {
			return mTypeInfo;
		}

		bool operator==(const Type& rhs) const noexcept {
			return mTypeInfo == rhs.mTypeInfo;
		}

	private:
		const Impl::TypeInfo* mTypeInfo = nullptr;
	};

	///////////////////////////////////////////////////////////////////////////////
	// Info Factory
	///////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline Impl::TypeInfo* Impl::InfoFactory<T>::Resolve()noexcept
	{
		if (mTypeInfo == nullptr)
		{
			static TypeInfo typeInfo{
				DEFAULT_UID,
				std::is_class_v<T>,
				std::is_array_v<T>,
				std::extent_v<T>,
				nullptr,
				{},
				nullptr,
				[]()noexcept ->Type {
					return Type(&typeInfo);
				}
			};
			mTypeInfo = &typeInfo;
		}
		return mTypeInfo;
	}

}
}