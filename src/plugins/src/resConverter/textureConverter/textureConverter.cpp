#include "textureConverter.h"
#include "image.h"
#include "textureCompressor.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "core\helper\enumTraits.h"
#include "renderer\textureImpl.h"
#include "renderer\texture.h"
#include "imguiRhi\manager.h"
#include "imguiRhi\imguiEx.h"

namespace Cjing3D
{
	namespace 
	{
		void ConverterMetaDataToSamplerDesc(const TextureMetaObject& meta, GPU::SamplerDesc& samplerDesc)
		{
			samplerDesc.mAddressU = meta.mWrapModeU == TextureMetaObject::WrapMode::CLAMP ? GPU::TEXTURE_ADDRESS_CLAMP : GPU::TEXTURE_ADDRESS_BORDER;
			samplerDesc.mAddressV = meta.mWrapModeV == TextureMetaObject::WrapMode::CLAMP ? GPU::TEXTURE_ADDRESS_CLAMP : GPU::TEXTURE_ADDRESS_BORDER;
			samplerDesc.mAddressW = meta.mWrapModeW == TextureMetaObject::WrapMode::CLAMP ? GPU::TEXTURE_ADDRESS_CLAMP : GPU::TEXTURE_ADDRESS_BORDER;
			switch (meta.mFilter)
			{
			case TextureMetaObject::Filter::POINT:
				samplerDesc.mFilter = GPU::FILTER_MIN_MAG_MIP_POINT;
				break;
			case TextureMetaObject::Filter::LINEAR:
				samplerDesc.mFilter = GPU::FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case TextureMetaObject::Filter::ANISOTROPIC:
				samplerDesc.mFilter = GPU::FILTER_ANISOTROPIC;
				break;
			}
		}
	}

	void TextureMetaObject::Serialize(JsonArchive& archive)const
	{
		archive.Write("format", mFormat);
		archive.Write("generateMipmap", mGenerateMipmap);
		archive.Write("normalmap", mIsNormalMap);
	}

	void TextureMetaObject::Unserialize(JsonArchive& archive) 
	{
		archive.Read("format", mFormat);
		archive.Read("generateMipmap", mGenerateMipmap);
		archive.Read("normalmap", mIsNormalMap);
	}

	bool TextureResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		TextureMetaObject metaData = context.GetMetaData<TextureMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);

		// load image from src file
		MaxPathString ext;
		Path::GetPathExtension(Span(src, StringLength(src)), ext.toSpan());
		Image img = Image::Load(source.data(), source.size(), ext.c_str());
		if (!img)
		{
			Logger::Warning("[TextureConverter] failed to load image:%s.", src);
			return false;
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		// Converted file format:
		// | texture desc
		// | texture data
		MemoryStream stream;
		stream.Reserve(256);

		GPU::FormatInfo formatInfo = GPU::GetFormatInfo(img.GetFormat());
		bool hasAlpah = formatInfo.mABits > 0;

		// 1. texture general header
		GPU::TextureDesc texDesc = {};
		texDesc.mType = GPU::TEXTURE_2D;
		texDesc.mWidth = std::max(formatInfo.mBlockW, img.GetWidth());
		texDesc.mHeight = std::max(formatInfo.mBlockH, img.GetHeight());
		texDesc.mDepth = img.GetDepth();
		texDesc.mMipLevels = img.GetMipLevels();
		texDesc.mArraySize = 1;
		texDesc.mFormat = img.GetFormat();
		texDesc.mUsage = GPU::USAGE_IMMUTABLE;
		texDesc.mBindFlags = GPU::BIND_SHADER_RESOURCE;

		GPU::SamplerDesc samplerDesc = {};
		ConverterMetaDataToSamplerDesc(metaData, samplerDesc);

		TextureGeneralHeader header;
		header.mTexDesc = texDesc;
		header.mSamplerDesc = samplerDesc;
		CopyString(header.mFileType, "lbc");
		stream.Write(&header, sizeof(TextureGeneralHeader));

		// 2. compile image data
		TextureCompressor::Options options;
		options.mIsGenerateMipmaps = metaData.mGenerateMipmap;
		if (!TextureCompressor::Compress(img, options, stream))
		{
			Logger::Warning("[TextureConverter] failed to compile image:%s.", src);
			return false;
		}

		// 3. write resource
		if (!context.WriteResource(dest, stream.data(), stream.Size())) {
			return false;
		}

		context.AddOutput(dest);
		context.SetMetaData<TextureMetaObject>(metaData);
		return true;
	}

	void TextureResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
		Texture* texture = static_cast<Texture*>(res);
		if (!texture) {
			return;
		}
		auto& texDesc = texture->GetDesc();

		ImGuiEx::VLeftLabel("Size");
		ImGui::Text("%dx%d", texDesc.mWidth, texDesc.mHeight);
		ImGuiEx::VLeftLabel("Mip");
		ImGui::Text("%d", texDesc.mMipLevels);
		ImGuiEx::VLeftLabel("Format");
		ImGui::Text("%s", EnumTraits::EnumToName(texDesc.mFormat).data());

		if (mCurrentTexture != texture->GetHandle()) 
		{
			mCurrentTexture = texture->GetHandle();
			mCurrentTextureMeta = context.GetMetaData<TextureMetaObject>();
		}

		// show texture preview
		if (texture->GetHandle())
		{
			ImVec2 texture_size(200, 200);
			if (texDesc.mWidth > texDesc.mHeight) {
				texture_size.y = texture_size.x * texDesc.mHeight / texDesc.mWidth;
			}
			else {
				texture_size.x = texture_size.y * texDesc.mWidth / texDesc.mHeight;
			}

			if (mCurrentTexture != GPU::ResHandle::INVALID_HANDLE) {
				ImGui::Image((ImTextureID)&mCurrentTexture, texture_size);
			}

			Path fullPath(context.GetFileSystem().GetBasePath());
			fullPath.AppendPath(res->GetPath());
			if (ImGui::Button("Open externally"))
			{
				if (!Platform::ShellExecuteOpen(fullPath.c_str(), nullptr)) {
					Logger::Warning("Failed to open %s in exeternal editor.", fullPath.c_str());
				}
			}
		}

		// show image meta editor
		ImGui::Separator();

		if (ImGui::Button("Apply")) {
			context.SetMetaData<TextureMetaObject>(mCurrentTextureMeta);
		}
	}

	bool TextureResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Texture") && 
			(EqualString(ext, "jpg") ||
			 EqualString(ext, "png") ||
			 EqualString(ext, "jpeg") ||
			 EqualString(ext, "dds"));
	}
}