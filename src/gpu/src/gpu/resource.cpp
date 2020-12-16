#include "resource.h"
#include "gpu.h"

namespace Cjing3D
{
namespace GPU
{
	ResHandle ResHandle::INVALID_HANDLE;

	bool ResHandle::IsValid() const
	{
		return GPU::IsHandleValid(*this);
	}
}
}

