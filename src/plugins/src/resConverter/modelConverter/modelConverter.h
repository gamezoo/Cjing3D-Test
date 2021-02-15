#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"
#include "core\container\hashMap.h"
#include "modelImporter.h"

namespace Cjing3D
{
	class ModelMetaObject : public SerializedObject
	{
	public:
		virtual void Serialize(JsonArchive& archive)const;
		virtual void Unserialize(JsonArchive& archive);
	};

	class ModelResConverter : public IResConverter
	{
	private:
		HashMap<String, ModelImporter*> mImporters;

	public:
		ModelResConverter();
		~ModelResConverter()
		{
			for (auto kvp : mImporters) {
				CJING_SAFE_DELETE(kvp.second);
			}
			mImporters.clear();
		}

		void RegisterImporter(const char* ext, ModelImporter* importer)
		{
			mImporters.insert(ext, importer);
		}

		ModelImporter* GetImporter(const char* ext)
		{
			auto it = mImporters.find(ext);
			return it != nullptr ? *it : nullptr;
		}

		bool SupportsFileExt(const char* ext);
		bool SupportsType(const char* ext, const ResourceType& type)override;
		bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) override;
	};
}