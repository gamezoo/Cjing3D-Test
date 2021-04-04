#include "materialConverter.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "renderer\materialImpl.h"

namespace Cjing3D
{
	// material metadata
	class MaterialMetaObject : public SerializedObject
	{
	public:
		void Serialize(JsonArchive& archive)const override {}	
		void Unserialize(JsonArchive& archive)override {}
	};

	// material import data
	class ImportMaterial : public SerializedObject
	{
	public:
		void Serialize(JsonArchive& archive)const override
		{
			archive.Write("shaderType", mShaderType);
			archive.Write("shaderPath", mShaderPath);
			archive.WriteCallback("textures", [this](JsonArchive& archive) {
				for (int i = 0; i < mTextures.size(); i++) {
					archive.WriteAndPush(mTextures[i]);
				}
			});
		}

		void Unserialize(JsonArchive& archive)override
		{
			archive.Read("shaderType", mShaderType);
			archive.Read("shaderPath", mShaderPath);
			archive.Read("textures", [this](JsonArchive& archive) {
				for (int i = 0; i < mTextures.size(); i++) {
					archive.Read(i, mTextures[i]);
				}
			});
		}

	public:
		Material::ShaderType mShaderType = Material::ShaderType_Standard;
		String mShaderPath;
		StaticArray<String, (I32)Material::TextureSlot_Count> mTextures;
	};

	bool MaterialResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		MaterialMetaObject metaData = context.GetMetaData<MaterialMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);

		// 1. parse import material
		JsonArchive archive(ArchiveMode::ArchiveMode_Read, source.data(), source.size());
		if (archive.IsOpen()) {
			return false;
		}
		ImportMaterial importMaterial;
		importMaterial.Unserialize(archive);

		// 2. setup material data
		DynamicArray<MaterialTexture>  materialTextures;
		MaterialData materialData;
		materialData.mShaderType = importMaterial.mShaderType;
		CopyString(materialData.mShaderPath, importMaterial.mShaderPath);
		for (I32 i = 0; i < Material::TextureSlot_Count; i++) {
			CopyString(materialData.mTextures[i].mPath, importMaterial.mTextures[i]);
		}

		// 3. write
		File* file = CJING_NEW(File);
		if (!fileSystem.OpenFile(dest, *file, FileFlags::DEFAULT_WRITE))
		{
			CJING_SAFE_DELETE(file);
			return false;
		}
		file->Write(&materialData, sizeof(materialData));

		context.AddOutput(dest);
		context.SetMetaData<MaterialMetaObject>(metaData);
		return true;
	}

	void MaterialResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
	}

	bool MaterialResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Material") && EqualString(ext, "mat");
	}
}