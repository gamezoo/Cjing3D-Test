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

	class ModelImporterOBJImpl
	{
	public:
		struct ImportMaterial
		{
			String mPath;
		};

		struct ImportMesh
		{
			I32 mIndex = -1;
			String mName;
			DynamicArray<GPU::VertexElement> mVertexElements;
			I32 mVertices = 0;
			MemoryStream mVertexData;
			DynamicArray<I32> mIndices;
			DynamicArray<ModelSubMesh> mSubMeshes;
		};
		DynamicArray<ImportMesh> mMeshes;
		DynamicArray<ImportMaterial> mMaterials;
		DynamicArray<MeshInstData> mMeshInstDatas;

	private:
		ImportMesh& GetImportMesh(GPU::VertexElement* elements, I32 num, I32 indexStride);
		void GatherVertice(tinyobj::index_t& index, GPU::VertexElement* elements, I32 num, MemoryStream& vertexData);

	public:
		bool Import(ResConverterContext& context, Span<char> memBuffer, const char* src);
		bool WriteModel(MemoryStream& stream);
	};

	ModelImporterOBJImpl::ImportMesh& ModelImporterOBJImpl::GetImportMesh(GPU::VertexElement* elements, I32 num, I32 indexStride)
	{
		// TODO: 在此处插入 return 语句
	}

	void ModelImporterOBJImpl::GatherVertice(tinyobj::index_t& index, GPU::VertexElement* elements, I32 num, MemoryStream& vertexData)
	{
	/*	for (int i = 0; i < num; i++)
		{
			GPU::VertexElement& element = elements[i];
			U32 stride = GPU::GetFormatInfo(element.mFormat).mBlockBits / 8;
		}*/
	}

	bool ModelImporterOBJImpl::Import(ResConverterContext& context, Span<char> memBuffer, const char* src)
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
		for (auto& objMaterial : objMaterials)
		{
		}

		// load meshes
		for (auto& shape : objShapes)
		{
			HashMap<I32, I32> registeredMaterialIndices;
			HashMap<U32, U32> uniqueVertices;

			DynamicArray<GPU::VertexElement> elements;
			ImportMesh& mesh = GetImportMesh(elements.data(), elements.size(), 4);

			for (I32 i = 0; i < shape.mesh.indices.size(); i += 3)
			{
				tinyobj::index_t reorderedIndices[] = {
					shape.mesh.indices[i + 0],
					shape.mesh.indices[i + 1],
					shape.mesh.indices[i + 2],
				};

				for (auto& index : reorderedIndices)
				{
					// this indexes the material library
					int materialIndex = std::max(0, shape.mesh.material_ids[i / 3]);
					if (!registeredMaterialIndices.find(materialIndex))
					{
						registeredMaterialIndices.insert(materialIndex, (I32)mMeshes.size());
						mesh.mSubMeshes.push(ModelSubMesh());
						mesh.mSubMeshes.back().mIndexOffset = (uint32_t)mesh.mIndices.size();

						// add new meshInstData
						MeshInstData& meshInst = mMeshInstDatas.emplace();
						meshInst.mMeshIndex = mesh.mIndex;
						meshInst.mSubMeshIndex = mesh.mSubMeshes.size() - 1;
						CopyString(meshInst.mMaterial, mMaterials[materialIndex].mPath.c_str());
					}

					// obj only support vertex/normal/texcoord
					U32 vertexHash = 0;
					HashCombine(vertexHash, index.vertex_index);
					HashCombine(vertexHash, index.normal_index);
					HashCombine(vertexHash, index.texcoord_index);
					HashCombine(vertexHash, materialIndex);

					if (uniqueVertices.find(vertexHash) == nullptr)
					{
						uniqueVertices.insert(vertexHash, (U32)mesh.mVertices);
						GatherVertice(index, elements.data(), elements.size(), mesh.mVertexData);
						mesh.mVertices++;
					}
					mesh.mIndices.push(uniqueVertices[vertexHash]);
					mesh.mSubMeshes.back().mIndices++;
				}
			}
		}

		return true;
	}

	bool ModelImporterOBJImpl::WriteModel(MemoryStream& stream)
	{
		// header
		ModelGeneralHeader generalHeader;
		generalHeader.mNumMeshes = mMeshes.size();
		generalHeader.mNumMeshInstDatas = mMeshInstDatas.size();
		stream.Write(&generalHeader, sizeof(ModelGeneralHeader));

		// meshInstDatas
		for (const auto& inst : mMeshInstDatas) {
			stream.Write(&inst, sizeof(MeshInstData));
		}

		// meshes
		I32 numVertexElements = 0;
		I32 numSubMeshes = 0;
		for (const auto& mesh : mMeshes)
		{
			ModelMeshData meshData;

			// calculate vertex size
			I32 vertexSize = 0;
			for (const auto& element : mesh.mVertexElements) {
				vertexSize += GPU::GetFormatInfo(element.mFormat).mBlockBits / 8;
			}

			CopyString(meshData.mName, mesh.mName.c_str());
			meshData.mVertexSize = vertexSize;
			meshData.mVertices = mesh.mVertices;
			meshData.mIndices = mesh.mIndices.size();
			meshData.mStartVertexElements = numVertexElements;
			meshData.mNumVertexElements = mesh.mVertexElements.size();
			meshData.mStartSubMeshes = numSubMeshes;
			meshData.mNumSubMeshes = mesh.mSubMeshes.size();

			stream.Write(&meshData, sizeof(meshData));

			numVertexElements += mesh.mVertexElements.size();
			numSubMeshes += mesh.mSubMeshes.size();
		}

		// write vertex elements
		for (const auto& mesh : mMeshes)
		{
			for (const auto& element : mesh.mVertexElements) {
				stream.Write(&element, sizeof(element));
			}
		}
		// write sub meshes
		for (const auto& mesh : mMeshes)
		{
			for (const auto& subMesh : mesh.mSubMeshes) {
				stream.Write(&subMesh, sizeof(subMesh));
			}
		}

		// write vertices + indices
		for (const auto& mesh : mMeshes) {
			stream.Write(mesh.mVertexData.data(), mesh.mVertexData.Size());
		}
		for (const auto& mesh : mMeshes) {
			stream.Write(mesh.mIndices.data(), sizeof(I32) * mesh.mIndices.size());
		}

		return true;
	}

	ModelImporterOBJ::ModelImporterOBJ()
	{
		mImpl = CJING_NEW(ModelImporterOBJImpl)();
	}

	ModelImporterOBJ::~ModelImporterOBJ()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	bool ModelImporterOBJ::SupportsFileExt(const char* ext)
	{
		return EqualString(ext, "obj");
	}

	bool ModelImporterOBJ::Import(ResConverterContext& context, Span<char> memBuffer, const char* src)
	{
		return mImpl->Import(context, memBuffer, src);
	}

	bool ModelImporterOBJ::WriteModel(MemoryStream& stream)
	{
		return mImpl->WriteModel(stream);
	}
}