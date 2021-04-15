#pragma once

#include "texture.h"
#include "math\color.h"

namespace Cjing3D
{
	namespace TextureHelper
	{
		void Initialize();
		void Uninitialize();

		Texture* GetWhite();
		Texture* GetBlack();
		Texture* GetColor(Color4 color);
		bool CreateTexture(Texture& texture, const U8* data, U32 width, U32 height, GPU::FORMAT format, const char* name = nullptr);
		bool CreateTexture(Texture& texture, const GPU::TextureDesc& desc, const U8* data = nullptr, const char* name = nullptr);
	}
}