#include "texture.h"
#include "textureImpl.h"
#include "resource\resourceManager.h"
#include "core\helper\enumTraits.h"

#define TINYDDSLOADER_IMPLEMENTATION
#include "tiny\tinyddsloader.h"

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

		GPU::FORMAT ConvertDDSFormatToFormat(tinyddsloader::DDSFile::DXGIFormat format)
		{
			GPU::FORMAT targetFormat = GPU::FORMAT_UNKNOWN;
			switch (format)
			{
			case tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm: targetFormat = GPU::FORMAT_B8G8R8A8_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB: targetFormat = GPU::FORMAT_B8G8R8A8_UNORM_SRGB; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm: targetFormat = GPU::FORMAT_R8G8B8A8_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB: targetFormat = GPU::FORMAT_R8G8B8A8_UNORM_SRGB; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UInt: targetFormat = GPU::FORMAT_R8G8B8A8_UINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_SNorm: targetFormat = GPU::FORMAT_R8G8B8A8_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_SInt: targetFormat = GPU::FORMAT_R8G8B8A8_SINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16G16_Float: targetFormat = GPU::FORMAT_R16G16_FLOAT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16G16_UNorm: targetFormat = GPU::FORMAT_R16G16_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16G16_UInt: targetFormat = GPU::FORMAT_R16G16_UINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16G16_SNorm: targetFormat = GPU::FORMAT_R16G16_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16G16_SInt: targetFormat = GPU::FORMAT_R16G16_SINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::D32_Float: targetFormat = GPU::FORMAT_D32_FLOAT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R32_Float: targetFormat = GPU::FORMAT_R32_FLOAT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R32_UInt: targetFormat = GPU::FORMAT_R32_UINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R32_SInt: targetFormat = GPU::FORMAT_R32_SINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8_UNorm: targetFormat = GPU::FORMAT_R8G8_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8_UInt: targetFormat = GPU::FORMAT_R8G8_UINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8_SNorm: targetFormat = GPU::FORMAT_R8G8_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8G8_SInt: targetFormat = GPU::FORMAT_R8G8_SINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16_Float: targetFormat = GPU::FORMAT_R16_FLOAT; break;
			case tinyddsloader::DDSFile::DXGIFormat::D16_UNorm: targetFormat = GPU::FORMAT_D16_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16_UNorm: targetFormat = GPU::FORMAT_R16_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16_UInt: targetFormat = GPU::FORMAT_R16_UINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16_SNorm: targetFormat = GPU::FORMAT_R16_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R16_SInt: targetFormat = GPU::FORMAT_R16_SINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8_UNorm: targetFormat = GPU::FORMAT_R8_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8_UInt: targetFormat = GPU::FORMAT_R8_UINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8_SNorm: targetFormat = GPU::FORMAT_R8_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::R8_SInt: targetFormat = GPU::FORMAT_R8_SINT; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC1_UNorm: targetFormat = GPU::FORMAT_BC1_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC1_UNorm_SRGB: targetFormat = GPU::FORMAT_BC1_UNORM_SRGB; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC2_UNorm: targetFormat = GPU::FORMAT_BC2_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC2_UNorm_SRGB: targetFormat = GPU::FORMAT_BC2_UNORM_SRGB; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC3_UNorm: targetFormat = GPU::FORMAT_BC3_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC3_UNorm_SRGB: targetFormat = GPU::FORMAT_BC3_UNORM_SRGB; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC4_UNorm: targetFormat = GPU::FORMAT_BC4_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC4_SNorm: targetFormat = GPU::FORMAT_BC4_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC5_UNorm: targetFormat = GPU::FORMAT_BC5_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC5_SNorm: targetFormat = GPU::FORMAT_BC5_SNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC7_UNorm: targetFormat = GPU::FORMAT_BC7_UNORM; break;
			case tinyddsloader::DDSFile::DXGIFormat::BC7_UNorm_SRGB: targetFormat = GPU::FORMAT_BC7_UNORM_SRGB; break;
			default:
				break;
			}
			return targetFormat;
		}

		GPU::ResHandle LoadDDS(Resource* resource, GPU::TextureDesc& texDesc, File& file, const char* name)
		{
			// TODO: create a mapping file to get data address?
			I64 bytes = file.Size() - file.Tell();
			U8* texData = CJING_NEW_ARR(U8, bytes);
			if (!file.Read(texData, bytes))
			{
				CJING_SAFE_DELETE_ARR(texData, bytes);
				return GPU::ResHandle::INVALID_HANDLE;
			}

			tinyddsloader::DDSFile ddsFile;
			auto result = ddsFile.Load(texData, bytes);
			if (result != tinyddsloader::Result::Success)
			{
				CJING_SAFE_DELETE_ARR(texData, bytes);
				return GPU::ResHandle::INVALID_HANDLE;
			}

			GPU::TextureDesc desc = texDesc;
			desc.mWidth = ddsFile.GetWidth();
			desc.mHeight = ddsFile.GetHeight();
			desc.mDepth = ddsFile.GetDepth();
			desc.mArraySize = ddsFile.GetArraySize();
			desc.mMipLevels = ddsFile.GetMipCount();
			if (ddsFile.IsCubemap()) {
				desc.mMiscFlags |= GPU::RESOURCE_MISC_TEXTURECUBE;
			}

			GPU::FORMAT targetFormat = ConvertDDSFormatToFormat(ddsFile.GetFormat());
			if (targetFormat == GPU::FORMAT_UNKNOWN)
			{
				Logger::Warning("Invalid dds texture format:%s", EnumTraits::EnumToName(desc.mFormat).data());
				CJING_SAFE_DELETE_ARR(texData, bytes);
				return GPU::ResHandle::INVALID_HANDLE;
			}
			desc.mFormat = targetFormat;

			// create gpu::texture
			DynamicArray<GPU::SubresourceData> initalData;
			initalData.reserve(desc.mMipLevels * desc.mArraySize);

			for (U32 arrayIndex = 0; arrayIndex < desc.mArraySize; arrayIndex++)
			{
				for (U32 mipLevel = 0; mipLevel < desc.mMipLevels; mipLevel++)
				{
					auto imageData = ddsFile.GetImageData(mipLevel, arrayIndex);
					GPU::SubresourceData& subresourceData = initalData.emplace();
					subresourceData.mSysMem = imageData->m_mem;;
					subresourceData.mSysMemPitch = imageData->m_memPitch;
					subresourceData.mSysMemSlicePitch = imageData->m_memSlicePitch;
				}
			}

			GPU::ResHandle ret = GPU::CreateTexture(&desc, initalData.data(), name);
			if (ret != GPU::ResHandle::INVALID_HANDLE) 
			{
				GPU::FORMAT prevFormat = texDesc.mFormat;
				texDesc = desc;
				texDesc.mFormat = prevFormat;
			}
			CJING_SAFE_DELETE_ARR(texData, bytes);
			return ret;
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

			// load texture general header
			TextureGeneralHeader generalHeader;
			if (!file.Read(&generalHeader, sizeof(TextureGeneralHeader)))
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

			GPU::TextureDesc& texDesc = generalHeader.mDesc;
			GPU::ResHandle texHandle;
			if (EqualString(generalHeader.mFileType, "dds"))
			{
				texHandle = LoadDDS(resource, texDesc, file, name);
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