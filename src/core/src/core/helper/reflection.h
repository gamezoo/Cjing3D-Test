#pragma once

#include "reflection_impl.h"

// simple runtime reflection system
// based on https://github.com/skypjack/meta.git
//
// Feature:
//	1.member variable
//	2.function
//	3.constuctor
//	4.destructor
//  5.base type
//
// Todo:
//  1.type convert

namespace Cjing3D
{
namespace Reflection
{
	///////////////////////////////////////////////////////////////////////////////
	// function
	///////////////////////////////////////////////////////////////////////////////
	template<typename T, typename... Args, size_t... Indexes>
	Any Construct(Any* const args, std::index_sequence<Indexes...>)
	{
		Any ret;
		auto realArgs = std::make_tuple((args + Indexes)->TryCast<Args>()...);
		if ((std::get<Indexes>(realArgs) && ...)) {
			ret = T{(*std::get<Indexes>(realArgs))... };
		}
		return ret;
	}

	template<typename T, auto Dtor>
	bool Destruct(Handle handle)
	{
		if (handle.GetType() != Impl::InfoFactory<T>::Resolve()) {
			return false;
		}

		std::invoke(Dtor, *Any(handle).TryCast<T>());
		return true;
	}

	template<bool IsConst, typename T, auto Var>
	bool VarSetter(Handle instance, size_t index, Any value)
	{
		bool ret = false;
		if constexpr (!IsConst)
		{
			// class member
			if constexpr (std::is_member_object_pointer_v<decltype(Var)>)
			{
				using VarT = RemoveCVR<decltype(std::declval<T>().*Var)>;

				T* obj = Any(instance).TryCast<T>();
				if constexpr (std::is_array_v<VarT>)
				{
					// VarT如果是ArrayType，则需要转换为移除Extent后的类型，并根据index set
					using RealT = std::remove_extent_t<VarT>;
					RealT* var = value.TryCast<RealT>();
					if (obj && var)
					{
						std::invoke(Var, obj)[index] = *var;
						ret = true;
					}
				}
				else
				{
					VarT* var = value.TryCast<VarT>();
					if (obj && var)
					{
						std::invoke(Var, obj) = *var;
						ret = true;
					}
				}

			}
			// static variable
			else
			{
				// 静态成员可视为全局指针对象，setter直接对指针赋值
				static_assert(std::is_pointer_v<decltype(Var)>);
				using VarT = RemoveCVR<decltype(*Var)>;

				if constexpr (std::is_array_v<VarT>)
				{
					// VarT如果是ArrayType，则需要转换为移除Extent后的类型，并根据index set
					using RealT = std::remove_extent_t<VarT>;
					RealT* var = value.TryCast<RealT>();
					if (var)
					{
						(*Var)[index] = *var;
						ret = true;
					}
				}
				else
				{
					VarT* var = value.TryCast<VarT>();
					if (var)
					{
						*Var = *var;
						ret = true;
					}
				}
			}
		}
		return ret;
	}

	template<typename T, auto Var>
	Any VarGetter(Handle instance, size_t index)
	{
		auto DispatchValue = [](auto&& value) {
			return Any(std::forward<decltype(value)>(value));
		};

		if constexpr (std::is_member_object_pointer_v<decltype(Var)>)
		{
			using VarT = RemoveCVR<decltype(std::declval<T>().*Var)>;
			T* obj = Any(instance).TryCast<T>();

			if constexpr (std::is_array_v<VarT>) {	
				return obj != nullptr ? DispatchValue(std::invoke(Var, obj)[index]) : Any();
			}
			else {
				return obj != nullptr ? DispatchValue(std::invoke(Var, obj)) : Any();
			}
		}
		else
		{
			static_assert(std::is_pointer_v<std::decay_t<decltype(Var)>>);
			if constexpr (std::is_array_v<std::remove_pointer_t<decltype(Var)>>) {
				return DispatchValue((*Var)[index]);
			}
			else {
				return DispatchValue(*Var);
			}
		}
		return Any();
	}

	template<typename T, auto Func, size_t... Indexes>
	Any Invoke([[maybe_unused]] Handle instance, Any* args, std::index_sequence<Indexes...>)
	{
		using FunctionHelperT = Impl::FunctionHelperT<decltype(Func)>;
		auto DispatchValue = [](auto *... args) {
			if constexpr (std::is_void_v<typename FunctionHelperT::RetType>)
			{
				std::invoke(Func, *args...);
				return Any{std::in_place_type_t<void> };
			}
			else
			{
				return Any{std::invoke(Func, *args...) };
			}
		};

		// 从AnyArray中获取类型转换后的ArgArray
		const auto realArgs = std::make_tuple(
			[](Any* any, auto* instance) {
				using ArgType = std::remove_reference_t<decltype(*instance)>;
				// try convert
				return instance;
			}(
				args + Indexes, 
			   (args + Indexes)->TryCast<std::tuple_element_t<Indexes, typename FunctionHelperT::ArgsType>>()
			)...
		);

		if constexpr (std::is_function_v<std::remove_pointer_t<decltype(Func)>>)
		{
			// 所有参数都转换成功了才执行对应函数
			return (std::get<Indexes>(realArgs) && ...) ? DispatchValue(std::get<Indexes>(realArgs)...) : Any{};
		}
		else
		{	
			auto* obj = Any(instance).TryCast<T>();
			return (obj && (std::get<Indexes>(realArgs) && ...)) ? DispatchValue(obj, std::get<Indexes>(realArgs)...) : Any{};
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// MetaFactory
	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	class MetaFactory
	{
	public:
		MetaFactory& BeginType(UID uid)
		{
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::Resolve();
			typeInfo->mIdentifier = uid;
			Impl::InfoFactory<T>::mTypeInfo = typeInfo;

			return *this;
		}

		template<typename BaseT>
		MetaFactory& Base() noexcept
		{
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::Resolve();
			static Impl::BaseInfo baseInfo {
				typeInfo,
				&Impl::InfoFactory<BaseT>::Resolve,
				[]()noexcept ->BaseType {
					return BaseType(&baseInfo);
				},
				[](void* instance)noexcept ->void* {
					return static_cast<BaseT*>(static_cast<T*>(instance));
				}
			};

			typeInfo->mBase = &baseInfo;
			return *this;
		}

		template<typename... Args>
		MetaFactory& AddCtor() noexcept
		{
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::Resolve();
			using FunctionHelperT = Impl::FunctionHelperT<T(*)(Args...)>;

			static Impl::CtorInfo ctorInfo{
				typeInfo,
				FunctionHelperT::ArgSize,
				&FunctionHelperT::ArgType,
				[](Any* const any) {
					return Construct<T, RemoveCVR<Args>...>(any, std::make_index_sequence<FunctionHelperT::ArgSize>());
				},
				[]()noexcept ->Constructor {
					return Constructor(&ctorInfo);
				},
			};

			typeInfo->mCtors.push_back(&ctorInfo);
			return *this;
		}

		template<auto Dtor>
		MetaFactory& AddDtor() noexcept
		{
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::Resolve();

			static Impl::DtorInfo dtorInfo{
				typeInfo,
				&Destruct<T, Dtor>,
				[]()noexcept ->Destructor {
					return Destructor(&dtorInfo);
				},
			};

			typeInfo->mDtor = &dtorInfo;
			return *this;
		}

		template<auto Func>
		MetaFactory& AddFunc(UID uid) noexcept
		{
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::Resolve();
			using FunctionHelperT = Impl::FunctionHelperT<decltype(Func)>;

			static Impl::FuncInfo funcInfo{
				uid,
				typeInfo,
				FunctionHelperT::IsConst,
				!std::is_member_function_pointer_v<decltype(Func)>,
				FunctionHelperT::ArgSize,
				Impl::InfoFactory<FunctionHelperT::RetType>::Resolve,
				&FunctionHelperT::ArgType,
				[](Handle handle, Any* any) {
					return Invoke<T, Func>(handle, any, std::make_index_sequence<FunctionHelperT::ArgSize>{});
				},
				[]()noexcept ->Function {
					return Function(&funcInfo);
				},
			};

			typeInfo->mFuncs.insert(std::make_pair(uid, &funcInfo));
			return *this;
		}

		template<auto Var>
		MetaFactory& AddVar(UID uid) noexcept
		{
			// auto Var会传入成员变量的指针，并创建Static VarInfo，其中包含了
			// Var的Getter和Setter函数

			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::Resolve();
			Impl::VarInfo* retInfo = nullptr;

			if constexpr (std::is_same_v<T, decltype(Var)>)
			{
				// do nothing...
				return *this;
			}
			else if constexpr (std::is_member_object_pointer_v<decltype(Var)>)
			{
				// 变量类型为非静态成员变量
				using VarT = std::remove_reference_t<decltype(std::declval<T>().*Var)>;
				static Impl::VarInfo varInfo{
					DEFAULT_UID,
					typeInfo,
					std::is_const_v<VarT>,
					!std::is_member_object_pointer_v<decltype(Var)>,
					& VarSetter<std::is_const_v<VarT>, T, Var>,
					& VarGetter<T, Var>,
					[]()noexcept ->Variable {
						return Variable(&varInfo);
					},
					Impl::InfoFactory<VarT>::Resolve
				};
				retInfo = &varInfo;
			}
			else
			{
				// 变量类型为指针变量、或静态成员变量
				static_assert(std::is_pointer_v<std::decay_t<decltype(Var)>>);
				using VarT = std::remove_pointer_t<decltype(Var)>;

				static Impl::VarInfo varInfo{
					DEFAULT_UID,
					typeInfo,
					std::is_const_v<VarT>,
					!std::is_member_object_pointer_v<decltype(Var)>,
					&VarSetter<std::is_const_v<VarT>, T, Var>,
					&VarGetter<T, Var>,
					[]()noexcept ->Variable {
						return Variable(&varInfo);
					},
					Impl::InfoFactory<VarT>::Resolve
				};
				retInfo = &varInfo;
			}

			retInfo->mIdentifier = uid;
			typeInfo->mVars.insert(std::make_pair(uid, retInfo));

			return *this;
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////
	// Common function
	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	MetaFactory<T> Reflect(UID uid)
	{
		return MetaFactory<T>().BeginType(uid);
	}

	template<typename T>
	inline Type Resolve()
	{
		return Impl::InfoFactory<T>::Resolve()->ToType();
	}

	inline Type Resolve(UID uid)
	{
		return Type();
	}
}

#define UID_HASH(str) Reflection::CalculateUIDHash(str)
}