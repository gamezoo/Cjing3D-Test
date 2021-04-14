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

	/// ///////////////////////////////////////////////////////////////////////
	/// Impl
	class ModelImporterOBJImpl
	{
	public:
		struct ImportMaterial
		{
			String mPath;
			tinyobj::material_t* mTinyMaterial = nullptr;
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

			tinyobj::shape_t* mTinyObjShape = nullptr;
		};
		DynamicArray<ImportMesh> mMeshes;
		DynamicArray<ImportMaterial> mMaterials;
		DynamicArray<MeshInstData> mMeshInstDatas;

		// tiny obj objects
		tinyobj::attrib_t objAttrib;
		std::vector<tinyobj::shape_t> objShapes;
		std::vector<tinyobj::material_t> objMaterials;

	public:
		bool Import(ResConverterContext& context, Span<char> memBuffer, const char* src);
		void PostprocessMeshes();
		bool WriteModel(MemoryStream& stream);
		bool WriteMaterials(const char* dirPath);
	};

	void ModelImporterOBJImpl::PostprocessMeshes()
	{
		// TODO: use jobsystem
		for (ImportMesh& importMesh : mMeshes)
		{
			tinyobj::shape_t& shape = *importMesh.mTinyObjShape;
			MemoryStream& vertexData = importMesh.mVertexData;

			for (auto& shape : objShapes)
			{
				HashMap<U32, U32> uniqueVertices;
				HashMap<I32, bool> registeredMaterialIndices;
				I32 currentSubMesh = -1;

				for (I32 i = 0; i < shape.mesh.indices.size(); i += 3)
				{
					tinyobj::index_t reorderedIndices[] = {
						shape.mesh.indices[i + 0],
						shape.mesh.indices[i + 1],
						shape.mesh.indices[i + 2],
					};

					for (auto& index : reorderedIndices)
					{
						int materialIndex = std::max(0, shape.mesh.material_ids[i / 3]);
						if (!registeredMaterialIndices.find(materialIndex))
						{
							registeredMaterialIndices.insert(materialIndex, true);
							currentSubMesh++;
						}

						// obj only support vertex/normal/texcoord
						U32 vertexHash = 0;
						HashCombine(vertexHash, index.vertex_index);
						HashCombine(vertexHash, index.normal_index);
						HashCombine(vertexHash, index.texcoord_index);
						HashCombine(vertexHash, materialIndex);

						if (uniqueVertices.find(vertexHash) == nullptr)
						{
							uniqueVertices.insert(vertexHash, (U32)importMesh.mVertices++);
							
							// position
							F32x3 pos = F32x3(
								objAttrib.vertices[index.vertex_index * 3 + 0],
								objAttrib.vertices[index.vertex_index * 3 + 1],
								objAttrib.vertices[index.vertex_index * 3 + 2]
							);
							vertexData.Write(pos);

							// normal
							if (!objAttrib.normals.empty())
							{
								F32x3 nor = F32x3(
									objAttrib.normals[index.normal_index * 3 + 0],
									objAttrib.normals[index.normal_index * 3 + 1],
									objAttrib.normals[index.normal_index * 3 + 2]
								);
								vertexData.Write(nor);
							}
							// texcoord
							if (index.texcoord_index >= 0 && !objAttrib.texcoords.empty())
							{
								F32x2 tex = F32x2(
									objAttrib.texcoords[index.texcoord_index * 2 + 0],
									1 - objAttrib.texcoords[index.texcoord_index * 2 + 1]
								);
								vertexData.Write(tex);
							}
						}
						importMesh.mIndices.push(uniqueVertices[vertexHash]);
						importMesh.mSubMeshes[currentSubMesh].mIndices++;
					}
				}
			}
		}
	}

	bool ModelImporterOBJImpl::Import(ResConverterContext& context, Span<char> memBuffer, const char* src)
	{
		// tiny objects
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

		// load vertex elements
		StaticArray<GPU::VertexElement, GPU::VERTEX_USAGE_COUNT> elements;
		I32 numElements = 0;
		// positoin
		elements[numElements++] = GPU::VertexElement::VertexData(GPU::VERTEX_USAGE_POSITION, GPU::FORMAT::FORMAT_R32G32B32_FLOAT);
		// normals
		if (!objAttrib.normals.empty()) {
			elements[numElements++] = GPU::VertexElement::VertexData(GPU::VERTEX_USAGE_NORMAL, GPU::FORMAT::FORMAT_R8G8B8A8_SNORM);
		}
		// texcoords
		if (objAttrib.texcoords.empty()) {
			elements[numElements++] = GPU::VertexElement::VertexData(GPU::VERTEX_USAGE_UV, GPU::FORMAT::FORMAT_R16G16_FLOAT);
		}
		// colors
		if (!objAttrib.colors.empty()) {
			elements[numElements++] = GPU::VertexElement::VertexData(GPU::VERTEX_USAGE_COLOR, GPU::FORMAT::FORMAT_R8G8B8A8_UNORM);
		}

		// load materials
		for (auto& objMaterial : objMaterials)
		{
			ImportMaterial& mat = mMaterials.emplace();
			mat.mPath = objMaterial.name;
			mat.mTinyMaterial = &objMaterial;

			// gather textures
			auto gatherTexture = [this, &mat, src]() {

			};

		}

		// load meshes
		for (auto& shape : objShapes)
		{
			ImportMesh& mesh = mMeshes.emplace();
			mesh.mTinyObjShape = &shape;
			mesh.mVertexElements.insert(elements.data(), elements.data() + numElements);

			HashMap<I32, I32> registeredMaterialIndices;

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
						registeredMaterialIndices.insert(materialIndex, (I32)mesh.mSubMeshes.size());
						mesh.mSubMeshes.push(ModelSubMesh());
						mesh.mSubMeshes.back().mIndexOffset = (uint32_t)mesh.mIndices.size();

						// add new meshInstData
						MeshInstData& meshInst = mMeshInstDatas.emplace();
						meshInst.mMeshIndex = mesh.mIndex;
						meshInst.mSubMeshIndex = mesh.mSubMeshes.size() - 1;
						CopyString(meshInst.mMaterial, mMaterials[materialIndex].mPath.c_str());
					}
				}
			}
		}

		return true;
	}

	bool ModelImporterOBJImpl::WriteModel(MemoryStream& stream)
	{
		// 1. postprocess meshes
		PostprocessMeshes();

		// 2. write model datas
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

		// 3. clear tiny objs
		objShapes.clear();
		objMaterials.clear();

		return true;
	}

	bool ModelImporterOBJImpl::WriteMaterials(const char* dirPath)
	{
		for (const auto& material : mMaterials)
		{

		}
		return true;
	}

	/// ///////////////////////////////////////////////////////////////////////
	/// ModelImporter
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

	bool ModelImporterOBJ::WriteMaterials(const char* dirPath)
	{
		return mImpl->WriteMaterials(dirPath);
	}
}