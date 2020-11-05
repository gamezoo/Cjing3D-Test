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
	class  Function;
	class  Constructor;
	class  Destructor;

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
			const TypeInfo* mParent;
			const size_t mArgSize;
			TypeInfo* (* const ArgType)(size_t)noexcept;
			Any(* const Invoke)(Any*);
			Constructor(* const ToCtor)() noexcept;
		};

		struct DtorInfo
		{
			const TypeInfo* mParent;
			bool(* const Invoke)(Handle);
			Destructor(* const ToDtor)() noexcept;
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
			const TypeInfo* mParent;
			const bool mIsConst;
			const bool mIsStatic;
			const size_t mArgSize;
			TypeInfo* (* const RetType)()noexcept;
			TypeInfo* (* const ArgType)(size_t)noexcept;
			Any(* const Invoke)(Handle, Any*);
			Function(* const ToFunc)() noexcept;
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
			bool (* const Compare)(const void*, const void*);
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

		inline bool CanCast(const TypeInfo* from, const TypeInfo* to)noexcept
		{
			return (from == to);
		}

		/////////////////////////////////////////////////////////////////////////////
		// Function Helper
		/////////////////////////////////////////////////////////////////////////////
		template<typename FuncT>
		struct FuncHelper;

		template<typename Ret, typename... Args>
		struct FuncHelper<Ret(Args...)>
		{
			using RetType = RemoveCVR<Ret>;
			using ArgsType = std::tuple<RemoveCVR<Args>...>;

			static constexpr bool IsConst = false;
			static constexpr size_t ArgSize = sizeof...(Args);

			static TypeInfo* ArgType(size_t index)noexcept
			{
				return std::array<TypeInfo*, sizeof...(Args)>{ {InfoFactory<Args>::Resolve()...}} [index] ;
			}
		};

		template<typename Ret, typename... Args>
		struct FuncHelper<Ret(Args...) const> : FuncHelper<Ret(Args...)>
		{
			static constexpr bool IsConst = true;
		};

		template<typename Ret, typename... Args, typename T>
		constexpr FuncHelper<Ret(Args...)> GetFunctoinHelper(Ret(T::*)(Args...));

		template<typename Ret, typename... Args, typename T>
		constexpr FuncHelper<Ret(Args...)const> GetFunctoinHelper(Ret(T::*)(Args...)const);

		template<typename Ret, typename... Args>
		constexpr FuncHelper<Ret(Args...)> GetFunctoinHelper(Ret(*)(Args...));

		constexpr void GetFunctoinHelper(...);

		template<typename Func>
		using FunctionHelperT = decltype(GetFunctoinHelper(std::declval<Func>()));


		/////////////////////////////////////////////////////////////////////////////
		// Compare Helper
		/////////////////////////////////////////////////////////////////////////////
		// 如果T类型不是void类型，且不是function类型，则静态转换后比较
		// SFINAE判断当对象本身无法直接比较时，也使用不同的Compre函数
		template<typename T, typename = std::enable_if_t<!std::is_void_v<T> && !std::is_function_v<T>>>
		static auto Compare(int, const void* lhs, const void* rhs) 
			-> decltype(std::declval<T>() == std::declval<T>(), bool{})
		{
			return *static_cast<const T*>(lhs) == *static_cast<const T*>(rhs);
		}

		template<typename>
		static bool Compare(char, const void* lhs, const void* rhs)
		{
			return lhs == rhs;
		}

		/////////////////////////////////////////////////////////////////////////////
		// Ctor Helper
		/////////////////////////////////////////////////////////////////////////////
		template<typename... Args, size_t... Indexes>
		inline const CtorInfo* FindCtor(std::index_sequence<Indexes...>, const TypeInfo* typeInfo)
		{
			for (const CtorInfo* info : typeInfo->mCtors)
			{
				// 当参数数量一致，且每个参数类型都可转换时返回
				if (info->mArgSize == sizeof...(Args) &&
					(([](const TypeInfo* from, const TypeInfo* to) {
						return CanCast(from, to);
					}(Impl::InfoFactory<Args>::Resolve(), info->ArgType(Indexes))) && ...))
				{
					return info;
				}
			}
			return nullptr;
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

		bool operator==(const Any& rhs) const noexcept
		{
			return mTypeInfo == rhs.mTypeInfo && (!mTypeInfo || mTypeInfo->Compare(mInstance, rhs.mInstance));
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

		Type GetType()const noexcept;
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
	// Constructor
	///////////////////////////////////////////////////////////////////////////////
	// impl::CtorInfo wrapper
	class Constructor
	{
	public:
		Constructor() noexcept {}
		Constructor(const Impl::CtorInfo* ctorInfo) :
			mCtorInfo(ctorInfo)
		{}

		size_t ArgSize()const { return mCtorInfo->mArgSize; }

		template<typename... Args>
		Any Invoke(Args&&... args)
		{
			if (sizeof...(args) != ArgSize()) {
				return Any();
			}

			std::array<Any, sizeof...(Args)> anyArray{ {Handle(args)...} };
			return mCtorInfo->Invoke(anyArray.data());
		}

		explicit operator bool() const noexcept {
			return mCtorInfo;
		}

		bool operator==(const Constructor& rhs) const noexcept {
			return mCtorInfo == rhs.mCtorInfo;
		}

	private:
		const Impl::CtorInfo* mCtorInfo = nullptr;
	};

	///////////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////////
	// impl::CtorInfo wrapper
	class Destructor
	{
	public:
		Destructor() noexcept {}
		Destructor(const Impl::DtorInfo* dtorInfo) :
			mDtorInfo(dtorInfo)
		{}

		bool Invoke(Handle handle)
		{
			return mDtorInfo->Invoke(handle);
		}

		explicit operator bool() const noexcept {
			return mDtorInfo;
		}

		bool operator==(const Destructor& rhs) const noexcept {
			return mDtorInfo == rhs.mDtorInfo;
		}

	private:
		const Impl::DtorInfo* mDtorInfo = nullptr;
	};

	///////////////////////////////////////////////////////////////////////////////
	// Function
	///////////////////////////////////////////////////////////////////////////////
	// impl::FuncInfo wrapper
	class Function
	{
	public:
		Function() noexcept {}
		Function(const Impl::FuncInfo* varInfo) :
			mFuncInfo(varInfo)
		{}

		bool IsConst()const { return mFuncInfo->mIsConst; }
		bool IsStatic()const { return mFuncInfo->mIsStatic; }
		size_t ArgSize()const { return mFuncInfo->mArgSize; }

		template<typename... Args>
		Any Invoke(Handle handle, Args&&... args)
		{
			if (sizeof...(args) != ArgSize()) {
				return Any();
			}

			std::array<Any, sizeof...(Args)> anyArray{{Handle(args)...}};
			return mFuncInfo->Invoke(handle, anyArray.data());
		}

		explicit operator bool() const noexcept {
			return mFuncInfo;
		}

		bool operator==(const Function& rhs) const noexcept {
			return mFuncInfo == rhs.mFuncInfo;
		}

	private:
		const Impl::FuncInfo* mFuncInfo = nullptr;
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

		template<typename... Args>
		Constructor Ctor()const
		{
			const Impl::CtorInfo* info = Impl::FindCtor<Args...>(std::make_index_sequence<sizeof...(Args)>(), mTypeInfo);
			return info != nullptr ? info->ToCtor() : Constructor();
		}

		Destructor Dtor()const
		{
			return mTypeInfo->mDtor != nullptr ? mTypeInfo->mDtor->ToDtor() : Destructor();
		}

		Variable Var(UID uid)const
		{
			if (!mTypeInfo) {
				return Variable();
			}

			auto it = mTypeInfo->mVars.find(uid);
			return it != mTypeInfo->mVars.end() ? it->second->ToVar() : Variable();
		}

		Function Func(UID uid)const
		{
			if (!mTypeInfo) {
				return Function();
			}

			auto it = mTypeInfo->mFuncs.find(uid);
			return it != mTypeInfo->mFuncs.end() ? it->second->ToFunc() : Function();
		}

		explicit operator bool() const noexcept {
			return mTypeInfo;
		}

		bool operator==(const Type& rhs) const noexcept {
			return mTypeInfo == rhs.mTypeInfo;
		}

		bool operator!=(const Type& rhs) const noexcept {
			return mTypeInfo != rhs.mTypeInfo;
		}

	private:
		const Impl::TypeInfo* mTypeInfo = nullptr;
	};

	Type Handle::GetType() const noexcept
	{
		return mTypeInfo != nullptr ? mTypeInfo->ToType() : Type();
	}

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
				},
				[](const void* lhs, const void* rhs) {
					return Impl::Compare<T>(0, lhs, rhs);
				}
			};
			mTypeInfo = &typeInfo;
		}
		return mTypeInfo;
	}

}
}