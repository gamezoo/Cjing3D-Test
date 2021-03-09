#include "texture.h"
#include "resource\resourceManager.h"

namespace Cjing3D
{
	class TextureFactory : public ResourceFactory
	{
	public:
		virtual void RegisterExtensions()
		{
			ResourceManager::RegisterExtension("jpg",  Texture::ResType);
			ResourceManager::RegisterExtension("jpeg", Texture::ResType);
			ResourceManager::RegisterExtension("png",  Texture::ResType);
		}

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

			// load texture desc
			GPU::TextureDesc texDesc;
			if (!file.Read(&texDesc, sizeof(texDesc))) 
			{
				Logger::Warning("[Resource] Failed to load texture");
				return false;
			}

			// TODO: create a mapping file to get data address?
			I64 bytes = file.Size() - sizeof(texDesc);
			U8* data = CJING_NEW_ARR(U8, bytes);
			if (!file.Read(data, bytes))
			{
				CJING_SAFE_DELETE_ARR(data, bytes);
				Logger::Warning("[Resource] Failed to load texture");
				return false;
			}

			// create gpu::texture
			auto CreateTexture = [&](const U8* texData)->GPU::ResHandle {

				DynamicArray<GPU::SubresourceData> initalData;
				initalData.reserve(texDesc.mMipLevels * texDesc.mArraySize);

				GPU::FormatInfo formatInfo = GPU::GetFormatInfo(texDesc.mFormat);
				I64 texDataOffset = 0;
				for (U32 arrayIndex = 0; arrayIndex < texDesc.mArraySize; ++arrayIndex)
				{
					for (U32 mip = 0; mip < texDesc.mMipLevels; ++mip)
					{
						const U32 width  = std::max(1u, texDesc.mWidth  >> mip);
						const U32 height = std::max(1u, texDesc.mHeight >> mip);
						const U32 depth  = std::max(1u, texDesc.mDepth  >> mip);

						GPU::TextureLayoutInfo texLayoutInfo = GPU::GetTextureLayoutInfo(texDesc.mFormat, width, height);
						GPU::SubresourceData& subresourceData = initalData.emplace();
						subresourceData.mSysMem = texData + texDataOffset;
						subresourceData.mSysMemPitch = texLayoutInfo.mPitch;
						subresourceData.mSysMemSlicePitch = texLayoutInfo.mSlicePitch;
						
						texDataOffset += GPU::GetTextureSize(texDesc.mFormat, width, height, depth, mip);
					}
				}

				return GPU::CreateTexture(&texDesc, initalData.data(), name);
			};
			auto texHandle = CreateTexture(data);
			if (texHandle == GPU::ResHandle::INVALID_HANDLE)
			{
				CJING_SAFE_DELETE_ARR(data, bytes);
				Logger::Warning("[Resource] Failed to load texture");
				return false;
			}
			CJING_SAFE_DELETE_ARR(data, bytes);

			texture->SetTexture(texHandle, texDesc);

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

	Texture::Texture(Texture&& rhs)
	{
		mHandle = rhs.mHandle;
		mDesc = rhs.mDesc;
		rhs.mHandle = GPU::ResHandle::INVALID_HANDLE;
		rhs.mDesc = GPU::TextureDesc();
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

	Texture& Texture::operator=(Texture&& rhs)
	{
		mHandle = rhs.mHandle;
		mDesc = rhs.mDesc;
		rhs.mHandle = GPU::ResHandle::INVALID_HANDLE;
		rhs.mDesc = GPU::TextureDesc();
		return *this;
	}
}