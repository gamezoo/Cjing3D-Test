#pragma once

#include "reflection_impl.h"

// simple runtime reflection system
// based on https://github.com/skypjack/meta.git

namespace Cjing3D
{
namespace Reflection
{
	///////////////////////////////////////////////////////////////////////////////
	// function
	///////////////////////////////////////////////////////////////////////////////
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

		MetaFactory& AddCtor(UID uid)
		{
			return *this;
		}

		MetaFactory& AddDtor(UID uid)
		{
			return *this;
		}

		MetaFactory& AddFunc(UID uid)
		{
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

#define UID_HASH(str) Reflection::CalculateHash(str)
}