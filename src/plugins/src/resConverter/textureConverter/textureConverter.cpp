#include "textureConverter.h"
#include "image.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "renderer\texture.h"
#include "imguiRhi\manager.h"
#include "imguiRhi\imguiEx.h"
#include "core\helper\enumTraits.h"

#include "nvtt\include\nvtt.h"

namespace Cjing3D
{
	struct ImageOuputHander : nvtt::OutputHandler
	{
	private:
		File& mFile;

	public:
		ImageOuputHander(File& file) : 
			nvtt::OutputHandler(),
			mFile(file) {}

		bool writeData(const void* data, int size) override { 
			return mFile.Write(data, size);
		}
		void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {}
		void endImage() override {}
	};

	void TextureMetaObject::Serialize(JsonArchive& archive)const
	{
		archive.Write("format", mFormat);
		archive.Write("generateMipmap", mGenerateMipmap);
	}

	void TextureMetaObject::Unserialize(JsonArchive& archive) 
	{
		archive.Read("format", mFormat);
		archive.Read("generateMipmap", mGenerateMipmap);
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

		GPU::FormatInfo formatInfo = GPU::GetFormatInfo(img.GetFormat());
		bool hasAlpah = formatInfo.mABits > 0;

		// setup nvtt
		nvtt::Context nvttContext;
		nvtt::InputOptions nvttInput;
		nvttInput.setMipmapGeneration(metaData.mGenerateMipmap);
		nvttInput.setAlphaMode(hasAlpah ? nvtt::AlphaMode_Transparency : nvtt::AlphaMode_None);
		nvttInput.setNormalMap(false);
		nvttInput.setTextureLayout(nvtt::TextureType_2D, img.GetWidth(), img.GetHeight(), img.GetDepth());
		nvttInput.setMipmapData(img.GetMipData<U8>(0), img.GetWidth(), img.GetHeight());

		File file;
		if (!fileSystem.OpenFile(dest, file, FileFlags::DEFAULT_WRITE))
		{
			Logger::Warning("[TextureConverter] failed to write dest file:%s.", dest);
			return false;
		}

		// Converted file format:
		// | texture desc
		// | texture data

		// 1. texture desc
		GPU::TextureDesc texDesc = {};
		texDesc.mWidth = std::max(formatInfo.mBlockW, img.GetWidth());
		texDesc.mHeight = std::max(formatInfo.mBlockH, img.GetHeight());
		texDesc.mDepth = img.GetDepth();
		texDesc.mMipLevels = 1;
		texDesc.mArraySize = 1;
		texDesc.mFormat = img.GetFormat();
		texDesc.mUsage = GPU::USAGE_IMMUTABLE;
		texDesc.mBindFlags = GPU::BIND_SHADER_RESOURCE;

		file.Write(&texDesc, sizeof(texDesc));

		// 2. image data
		// setup nvtt options
		ImageOuputHander nvttOutputHandler(file);
		nvtt::OutputOptions nvttOuput;
		nvttOuput.setSrgbFlag(false);
		nvttOuput.setOutputHandler(&nvttOutputHandler);

		nvtt::CompressionOptions compression;
		compression.setFormat(hasAlpah ? nvtt::Format_DXT5 : nvtt::Format_DXT1);
		compression.setQuality(nvtt::Quality_Normal);

		if (!nvttContext.process(nvttInput, compression, nvttOuput))
		{
			Logger::Warning("[TextureConverter] failed to compile image:%s.", src);
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

		// show texture preview
		if (texture->GetHandle())
		{

		}

		Path fullPath(context.GetFileSystem().GetBasePath());
		fullPath.AppendPath(res->GetPath());
		if (ImGui::Button("Open externally"))
		{
			if (!Platform::ShellExecuteOpen(fullPath.c_str(), nullptr)) {
				Logger::Warning("Failed to open %s in exeternal editor.", fullPath.c_str());
			}
		}

		// show image meta editor
		//TextureMetaObject data = context.GetMetaData<TextureMetaObject>();
		ImGui::Separator();

		if (ImGui::Button("Apply"))
		{
			// context.SetMetaData(data);
			// context.WriteMetaData();
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

	LUMIX_PLUGIN_ENTRY(textureConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(TextureResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}