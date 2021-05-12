#include "resource.h"

namespace Cjing3D
{
	U32 HashFunc(U32 Input, const RenderGraphResource& Data)
	{
		return HashFunc(Input, Data.Hash());
	}

	U64 HashFunc(U64 Input, const RenderGraphResource& Data)
	{
		return HashFunc(Input, Data.Hash());
	}

	RenderGraphBlackboard::RenderGraphBlackboard()
	{
	}

	RenderGraphBlackboard::~RenderGraphBlackboard()
	{
	}

	void RenderGraphBlackboard::Put(const char* name, RenderGraphResource res)
	{
		mResMap.insert(StringID(name), res);
	}

	RenderGraphResource RenderGraphBlackboard::Get(const char* name) const
	{
		auto it = mResMap.find(StringID(name));
		if (it != nullptr) {
			return *it;
		}
		return RenderGraphResource();
	}

	void RenderGraphBlackboard::Remove(const char* name)
	{
		mResMap.erase(StringID(name));
	}

	void RenderGraphBlackboard::Clear()
	{
		mResMap.clear();
	}
}