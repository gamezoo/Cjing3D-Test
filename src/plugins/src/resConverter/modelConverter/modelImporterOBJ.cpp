#include "modelImporterOBJ.h"
#include "modelConverter.h"
#include "renderer\modelImpl.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny\tinyobjloader.h"

// TODO: replace it
#include <streambuf>

namespace Cjing3D
{
	struct MemStreamBuffer : std::streambuf
	{
		MemStreamBuffer(char* buf, size_t length) {
			setg(buf, buf, buf + length);
		}
	};

	// Custom material file reader:
	class MaterialFileReader : public tinyobj::MaterialReader
	{
	public:
		explicit MaterialFileReader() {}
		virtual ~MaterialFileReader() {}

		bool operator()(const std::string& matId,
			std::vector<tinyobj::material_t>* materials,
			std::map<std::string, int>* matMap,
			std::string* err) override
		{
			return true;
		}
	};
;

	ModelImporterOBJ::ModelImporterOBJ(){}

	ModelImporterOBJ::~ModelImporterOBJ(){}

	bool ModelImporterOBJ::SupportsFileExt(const char* ext)
	{
		return EqualString(ext, "obj");
	}

	bool ModelImporterOBJ::Import(ResConverterContext& context, Span<char> memBuffer, const char* src)
	{
		// tiny objects
		tinyobj::attrib_t objAttrib;
		std::vector <tinyobj::shape_t> objShapes;
		std::vector<tinyobj::material_t> objMaterials;
		std::string objErrors;

		// mem buffer
		MemStreamBuffer buffer(memBuffer.data(), memBuffer.length());
		std::istream in(&buffer);

		MaterialFileReader matFileReader;
		bool success = tinyobj::LoadObj(&objAttrib, &objShapes, &objMaterials, &objErrors, &in, &matFileReader, true);
		if (success == false) 
		{
			Logger::Warning("Failed to import model obj:%s, %s", src, objErrors.c_str());
			return false;
		}

		// load materials
		for (auto& obj_material : objMaterials)
		{
		}

		// load meshes
		for (auto& shape : objShapes)
		{

		}

		return false;
	}

	void ModelImporterOBJ::WriteModel(File& file)
	{
		// ModelGeneralHeader
		ModelGeneralHeader generalHeader;
		file.Write(&generalHeader, sizeof(ModelGeneralHeader));

	}
}