#pragma once

#include "gpu\gpu.h"
#include "resource\resource.h"
#include "resource\resRef.h"

namespace Cjing3D
{
	class Texture : public Resource
	{
	public:
		DECLARE_RESOURCE(Texture, "Texture")

		Texture();
		Texture(GPU::ResHandle handle, const GPU::TextureDesc& desc);
		Texture(Texture&& rhs);
		~Texture();

		bool IsLoaded()const {
			return mHandle != GPU::ResHandle::INVALID_HANDLE;
		}

		const GPU::TextureDesc& GetDesc()const {
			return mDesc;
		}
		GPU::ResHandle GetHandle()const {
			return mHandle;
		}
		const GPU::ResHandle* GetHandlePtr()const {
			return &mHandle;
		}

		void Clear();
		void SetTexture(GPU::ResHandle handle, const GPU::TextureDesc& desc);

		Texture& operator=(Texture&& rhs);

	private:
		friend class TextureFactory;

		Texture(const Texture& rhs) = delete;
		Texture& operator=(const Texture& rhs) = delete;

		GPU::ResHandle mHandle;
		GPU::TextureDesc mDesc;
	};
	using TextureRef = ResRef<Texture>;
}