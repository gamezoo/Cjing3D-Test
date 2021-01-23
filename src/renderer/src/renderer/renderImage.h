#pragma once

#include "definitions.h"
#include "gpu\commandList.h"

namespace Cjing3D
{
	struct ImageParams
	{
		enum Flags
		{
			NORMAL,
			FULLSCREEN = 1 << 1
		};
		U32 mFlags;
		BLENDMODE mBlendFlag;

		void EnableFullScreen() { mFlags |= FULLSCREEN; }
		bool IsFullScreen()const { return FLAG_ANY(mFlags, FULLSCREEN); }
	};

	namespace RenderImage
	{
		void Initialize();
		void Draw(const GPU::ResHandle& tex, const ImageParams& param, GPU::CommandList& cmd);
	}
}