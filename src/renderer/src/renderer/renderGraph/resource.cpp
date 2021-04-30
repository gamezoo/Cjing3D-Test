#include "resource.h"

namespace Cjing3D
{
	U32 HashFunc(U32 Input, const RenderGraphResource& Data)
	{
		return HashFunc(Input, Data.Index());
	}

	U64 HashFunc(U64 Input, const RenderGraphResource& Data)
	{
		return HashFunc(Input, Data.Index());
	}
}