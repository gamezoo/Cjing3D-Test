#include "textureConverter.h"
#include "image.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "core\helper\enumTraits.h"
#include "core\helper\stream.h"
#include "renderer\textureImpl.h"
#include "renderer\texture.h"
#include "imguiRhi\manager.h"
#include "imguiRhi\imguiEx.h"

#include "nvtt\include\nvtt.h"

namespace Cjing3D
{
	// TODO
	// 1. metadata editor
	// 2. more img format support

	struct ImageErrorHandler : nvtt::ErrorHandler
	{
		virtual ~ImageErrorHandler() {}

		void error(nvtt::Error e)override
		{
			Logger::Error(nvtt::errorString(e));
		}
	};

	struct ImageOuputHander : nvtt::OutputHandler
	{
	private:
		MemoryStream& mStream;

	public:
		ImageOuputHander(MemoryStream& stream) : 
			nvtt::OutputHandler(),
			mStream(stream) {}

		bool writeData(const void* data, int size) override { 
			return mStream.Write(data, size);
		}
		void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {}
		void endImage() override {}
	};

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

		TextureGeneralHeader header;
		header.mDesc = texDesc;
		CopyString(header.mFileType, "dds");
		stream.Write(&header, sizeof(TextureGeneralHeader));

		// 2. image data
		// setup nvtt options
		nvtt::Context nvttContext;
		nvtt::InputOptions nvttInput;
		if (metaData.mIsNormalMap) {
			nvttInput.setGamma(1.0f, 1.0f);
		}
		nvttInput.setMipmapGeneration(metaData.mGenerateMipmap);
		nvttInput.setAlphaCoverageMipScale(metaData.mMipScale, formatInfo.mChannels == 4 ? 3 : 0);
		nvttInput.setAlphaMode(hasAlpah ? nvtt::AlphaMode_Transparency : nvtt::AlphaMode_None);
		nvttInput.setNormalMap(metaData.mIsNormalMap);
		nvttInput.setTextureLayout(nvtt::TextureType_2D, img.GetWidth(), img.GetHeight(), img.GetDepth());
		nvttInput.setMipmapData(img.GetMipData<U8>(0), img.GetWidth(), img.GetHeight());

		ImageOuputHander nvttOutputHandler(stream);
		ImageErrorHandler nvttErrorHandler;
		nvtt::OutputOptions nvttOuput;
		nvttOuput.setSrgbFlag(false);
		nvttOuput.setContainer(nvtt::Container_DDS10);
		nvttOuput.setErrorHandler(&nvttErrorHandler);
		nvttOuput.setOutputHandler(&nvttOutputHandler);

		nvtt::CompressionOptions compression;
		compression.setFormat(metaData.mIsNormalMap ? nvtt::Format_BC5 : (hasAlpah ? nvtt::Format_BC7 : nvtt::Format_BC6));
		compression.setQuality(nvtt::Quality_Normal);

		if (!nvttContext.process(nvttInput, compression, nvttOuput))
		{
			Logger::Warning("[TextureConverter] failed to compile image:%s.", src);
			return false;
		}

		// write resource
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