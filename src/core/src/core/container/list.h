#pragma once

#include "core\memory\stlAllocator.h"

#include <list>

namespace Cjing3D
{
	template <typename T, typename A = STLAllocator<T, STLAllocPolicy> >
	using List = std::list<T, A>;
}