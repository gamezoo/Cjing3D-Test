#pragma once

#include "reflection_impl.h"

// simple runtime reflection system
// based on https://github.com/skypjack/meta.git

namespace Cjing3D
{
namespace Reflection
{
	///////////////////////////////////////////////////////////////////////////////////////
	// MetaFactory
	///////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	class MetaFactory
	{
	public:
		MetaFactory& BeginType(UID uid)
		{
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::GetOrCreate();
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
			Impl::TypeInfo* typeInfo = Impl::InfoFactory<T>::GetOrCreate();
			Impl::VarInfo* retNode = nullptr;

			if constexpr (std::is_member_object_pointer_v<decltype(Var)>)
			{
				// 变量类型为非静态成员变量
				using VarT = std::remove_reference_t<decltype(std::declval<T>().*Var)>;
				static Impl::VarInfo node {
					DEFAULT_UID,
					std::is_const_v<VarT>,
					!std::is_member_object_pointer_v<decltype(Var)>
				};
				retNode = &node;
			}
			else
			{
				// 变量类型为指针变量、或静态成员变量
				static_assert(std::is_pointer_v<std::decay_t<decltype(Var)>>);
				using VarT = std::remove_pointer_t<decltype(Var)>;

				static Impl::VarInfo node{
					DEFAULT_UID,
					std::is_const_v<VarT>,
					!std::is_member_object_pointer_v<decltype(Var)>
				};
				retNode = &node;
			}

			retNode->mIdentifier = uid;
			typeInfo->mVars.insert(std::make_pair(uid, retNode));

			return *this;
		}
	};

	template<typename T>
	MetaFactory<T> Reflect(UID uid)
	{
		return MetaFactory<T>().BeginType(uid);
	}
}

#define UID_HASH(str) Reflection::CalculateHash(str)
}