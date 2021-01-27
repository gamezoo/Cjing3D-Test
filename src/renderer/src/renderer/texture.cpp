#include "texture.h"
#include "resource\resourceManager.h"

namespace Cjing3D
{
	class TextureFactory : public ResourceFactory
	{
	public:
		virtual Resource* CreateResource()
		{
			Texture* texture = CJING_NEW(Texture);
			return texture;
		}

		virtual bool LoadResource(Resource* resource, const char* name, File& file)
		{
			Texture* texture = reinterpret_cast<Texture*>(resource);
			if (!texture || !file) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

			Logger::Info("[Resource] Texture loaded successful:%s.", name);
			return true;
		}

		virtual bool DestroyResource(Resource* resource)
		{
			if (resource == nullptr) {
				return false;
			}

			Texture* texture = reinterpret_cast<Texture*>(resource);
			CJING_DELETE(texture);
			return true;
		}

		virtual bool IsNeedConvert()const
		{
			return true;
		}
	};
	DEFINE_RESOURCE(Texture, "Texture");

	Texture::Texture()
	{
	}

	Texture::Texture(GPU::ResHandle handle, const GPU::TextureDesc& desc) :
		mHandle(handle),
		mDesc(desc)
	{
	}

	Texture::~Texture()
	{
		Clear();
	}

	void Texture::Clear()
	{
		if (mHandle != GPU::ResHandle::INVALID_HANDLE && GPU::IsInitialized()) 
		{
			GPU::DestroyResource(mHandle);
			mHandle = GPU::ResHandle::INVALID_HANDLE;
		}
		mDesc = GPU::TextureDesc();
	}

	void Texture::SetTexture(GPU::ResHandle handle, const GPU::TextureDesc& desc)
	{
		mHandle = handle;
		mDesc = desc;
	}
}