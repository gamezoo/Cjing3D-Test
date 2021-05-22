#include "texture.h"
#include "textureImpl.h"
#include "resource\resourceManager.h"
#include "core\helper\enumTraits.h"
#include "core\helper\stream.h"

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

		GPU::ResHandle LoadRaw(Resource* resource, GPU::TextureDesc& texDesc, InputMemoryStream& inputStream, const char* name)
		{
			return GPU::ResHandle::INVALID_HANDLE;
		}

		GPU::ResHandle LoadLBC(Resource* resource, GPU::TextureDesc& texDesc, InputMemoryStream& inputStream, const char* name)
		{
			// lbc header
			const TextureLBCHeader* lbcHeader = (const TextureLBCHeader*)(inputStream.data() + inputStream.Offset());
			if (lbcHeader == nullptr)
			{
				Logger::Error("Invalid texture:%s", name);
				return GPU::ResHandle::INVALID_HANDLE;
			}
			inputStream.AddOffset(sizeof(TextureLBCHeader));

			// texture data
			U32 bytes = inputStream.Size() - inputStream.Offset();
			U8* texData = CJING_NEW_ARR(U8, inputStream.Size() - inputStream.Offset());
			if (!inputStream.Read(texData, bytes))
			{
				CJING_SAFE_DELETE_ARR(texData, bytes);
				return GPU::ResHandle::INVALID_HANDLE;
			}

			GPU::TextureDesc desc = texDesc;
			GPU::FORMAT srcFormat = texDesc.mFormat;
			desc.mFormat = lbcHeader->mCompressedFormat;

			// create gpu::texture
			DynamicArray<GPU::SubresourceData> initalData;
			initalData.reserve(desc.mMipLevels * desc.mArraySize);

			// set subresource data
			GPU::FormatInfo formatInfo = GPU::GetFormatInfo(srcFormat);
			U8* mem = texData;
			U32 faces = (desc.mMiscFlags & GPU::RESOURCE_MISC_TEXTURECUBE) ? 6 : 1;
			for (U32 arrayIndex = 0; arrayIndex < desc.mArraySize; arrayIndex++)
			{
				for (U32 faceIndex = 0; faceIndex < faces; faceIndex++)
				{
					for (U32 mipLevel = 0; mipLevel < desc.mMipLevels; mipLevel++)
					{
						const U32 w = std::max(desc.mWidth  >> mipLevel, 1u);
						const U32 h = std::max(desc.mHeight >> mipLevel, 1u);
						GPU::SubresourceData& subresourceData = initalData.emplace();
						subresourceData.mSysMem = mem;
						subresourceData.mSysMemPitch = w * (formatInfo.mBlockBits >> 3);

						mem += w * h * (U32)(formatInfo.mBlockBits >> 3);
					}
				}
			}

			GPU::ResHandle ret = GPU::CreateTexture(&desc, initalData.data(), name);
			if (ret != GPU::ResHandle::INVALID_HANDLE) 
			{
				texDesc = desc;
				texDesc.mFormat = srcFormat;
			}
			CJING_SAFE_DELETE_ARR(texData, bytes);
			return ret;
		}

		virtual bool LoadResource(Resource* resource, const char* name, U64 size, const U8* data)
		{
			Texture* texture = reinterpret_cast<Texture*>(resource);
			if (!texture || size <= 0 || data == nullptr) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

			InputMemoryStream inputStream(data, (U32)size);

			// load texture general header
			TextureGeneralHeader generalHeader;
			if (!inputStream.Read(&generalHeader, sizeof(TextureGeneralHeader)))
			{
				Logger::Warning("[Resource] Failed to load texture");
				return false;
			}

			// check version
			if (generalHeader.mMajor != TextureGeneralHeader::MAJOR ||
				generalHeader.mMinor != TextureGeneralHeader::MINOR)
			{
				Logger::Warning("Texture version mismatch.");
				return false;
			}

			GPU::TextureDesc& texDesc = generalHeader.mTexDesc;
			GPU::ResHandle texHandle;
			if (EqualString(generalHeader.mFileType, "lbc"))
			{
				texHandle = LoadLBC(resource, texDesc, inputStream, name);
			}
			else if (EqualString(generalHeader.mFileType, "Raw"))
			{
				texHandle = LoadRaw(resource, texDesc, inputStream, name);
			}
			else
			{
				Logger::Warning("Unsupported texture type:%s", generalHeader.mFileType);
				return false;
			}

			if (texHandle == GPU::ResHandle::INVALID_HANDLE)
			{
				Logger::Warning("[Resource] Failed to load texture:%s", name);
				return false;
			}

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