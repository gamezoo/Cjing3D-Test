#include "textureConverter.h"
#include "image.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"

#include "nvtt\include\nvtt.h"

namespace Cjing3D
{
	void TextureMetaObject::Serialize(JsonArchive& archive)const
	{
		archive.Write("format", mFormat);
		archive.Write("generateMipmap", mGenerateMiplevels);
	}

	void TextureMetaObject::Unserialize(JsonArchive& archive) 
	{
		archive.Read("format", mFormat);
		archive.Read("generateMipmap", mGenerateMiplevels);
	}

	bool TextureResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		TextureMetaObject data = context.GetMetaData<TextureMetaObject>();

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

		if (img.GetFormat() == GPU::FORMAT_R8G8B8A8_UNORM)
		{

		}

		// write texture
		GPU::TextureDesc texDesc = {};

		File file;
		if (!fileSystem.OpenFile(dest, file, FileFlags::DEFAULT_WRITE))
		{
			Logger::Warning("[TextureConverter] failed to write dest file:%s.", dest);
			return false;
		}

		file.Write(&texDesc, sizeof(texDesc));
		U32 texSize = GPU::GetTextureSize(texDesc.mFormat, texDesc.mWidth, texDesc.mHeight, texDesc.mDepth, texDesc.mMipLevels); 
		file.Write(img.GetMipData<U8>(0), texSize);

		context.AddOutput(dest);
		context.SetMetaData<TextureMetaObject>(data);
		return true;
	}

	void TextureResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
	}

	bool TextureResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Texture");
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