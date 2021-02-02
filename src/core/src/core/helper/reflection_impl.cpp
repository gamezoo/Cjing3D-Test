#include "reflection_impl.h"

namespace Cjing3D
{
namespace Reflection
{
	Type Handle::GetType() const noexcept
	{
		return mTypeInfo != nullptr ? mTypeInfo->ToType() : Type();
	}

	Type BaseType::Parent()const noexcept
	{
		return mBaseInfo != nullptr ? mBaseInfo->mParent : Type();
	}

	Type BaseType::GetType()const noexcept
	{
		return mBaseInfo != nullptr ? mBaseInfo->GetTypeInfo() : Type();
	}
}
}