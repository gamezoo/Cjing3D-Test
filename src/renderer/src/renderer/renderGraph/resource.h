#pragma once

#include "renderer\definitions.h"

namespace Cjing3D
{
	class RenderGraphResource
	{
	public:
		RenderGraphResource() = default;
		RenderGraphResource(I32 index, I32 version = 0) :mIndex(index), mVersion(version) {}

		bool operator==(const RenderGraphResource& rhs) const {
			return mIndex == rhs.mIndex;
		}
		bool operator!=(const RenderGraphResource& rhs) const {
			return mIndex != rhs.mIndex;
		}
		bool operator< (const RenderGraphResource& rhs) const {
			return mIndex < rhs.mIndex;
		}
		operator bool()const {
			return mIndex != -1;
		}

		bool IsEmpty()const {
			return mIndex == -1;
		}
		I32 Index()const {
			return mIndex;
		}
		I32 Hash()const {
			return mIndex * 10000 + mVersion;
		}

		I32 mIndex = -1;
		I32 mVersion = 0;
	};

	U32 HashFunc(U32 Input, const RenderGraphResource& Data);
	U64 HashFunc(U64 Input, const RenderGraphResource& Data);
}
