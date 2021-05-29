#include "model.h"
#include "modelImpl.h"
#include "resource\resourceManager.h"
#include "core\helper\stream.h"

namespace Cjing3D
{
	const U32 ModelGeneralHeader::MAGIC = 0x149B6314;

	class ModelFactory : public ResourceFactory
	{
	public:
		virtual void RegisterExtensions()
		{
			ResourceManager::RegisterExtension("obj", Model::ResType);
			ResourceManager::RegisterExtension("fbx", Model::ResType);
			ResourceManager::RegisterExtension("gltf",Model::ResType);
		}

		virtual Resource* CreateResource()
		{
			Model* model = CJING_NEW(Model);
			return model;
		}

		bool ParseMesh(ModelImpl& impl, ModelGeneralHeader& header, Model& model, InputMemoryStream& inputStream)
		{
			if (!GPU::IsInitialized()) {
				return false;
			}

			if (header.mNumMeshes <= 0 || header.mNumMeshInstDatas <= 0) {
				return false;
			}

			// meshInstDatas
			DynamicArray<MeshInstData> meshInstDatas;
			meshInstDatas.resize(header.mNumMeshInstDatas);
			if (!inputStream.Read(meshInstDatas.data(), header.mNumMeshInstDatas * sizeof(MeshInstData)))
			{
				Logger::Warning("Failed to read MeshInstDatas");
				return false;
			}

			// meshes
			DynamicArray<ModelMeshData> modelMeshDatas;
			if (!inputStream.Read(modelMeshDatas.data(), header.mNumMeshes * sizeof(ModelMeshData)))
			{
				Logger::Warning("Failed to read ModelMeshDatas");
				return false;
			}

			int numVertexElements = 0;
			int numSubMeshes = 0;
			int numIndices = 0;
			int vertexDataSize = 0;
			for (auto& meshData : modelMeshDatas)
			{
				numVertexElements += meshData.mNumVertexElements;
				numSubMeshes += meshData.mNumSubMeshes;
				vertexDataSize += meshData.mVertexElementSize * meshData.mVertices;
				numIndices += meshData.mIndices;
			}

			// vertex elements
			DynamicArray<GPU::VertexElement> vertexElements;
			if (numVertexElements > 0)
			{
				vertexElements.resize(numVertexElements);
				if (!inputStream.Read(vertexElements.data(), numVertexElements * sizeof(GPU::VertexElement)))
				{
					Logger::Warning("Failed to read VertexElements");
					return false;
				}
			}

			// sub meshes
			DynamicArray<ModelSubMesh> subMeshes;
			if (numSubMeshes > 0)
			{
				subMeshes.resize(numSubMeshes);
				if (!inputStream.Read(subMeshes.data(), numSubMeshes * sizeof(ModelSubMesh)))
				{
					Logger::Warning("Failed to read ModelSubMeshes");
					return false;
				}
			}

			// vertices 
			if (vertexDataSize > 0)
			{
				impl.mVertexData.resize(vertexDataSize);
				if (!inputStream.Read(impl.mVertexData.data(), vertexDataSize))
				{
					Logger::Warning("Failed to read vertex data");
					return false;
				}
			}
			
			// indices
			if (numIndices > 0)
			{
				impl.mIndices.resize(numIndices);
				if (!inputStream.Read(impl.mIndices.data(), numIndices * sizeof(I32)))
				{
					Logger::Warning("Failed to read indices data");
					return false;
				}
			}

			// initialize model
			impl.mMeshes.resize(modelMeshDatas.size());
			for (ModelMeshData& meshData : modelMeshDatas)
			{

			}

			for (MeshInstData& instData : meshInstDatas)
			{
				ModelMesh& mesh = impl.mMeshes[instData.mMeshIndex];
				ModelSubMesh& subMesh = subMeshes[instData.mSubMeshIndex];
				mesh.mSubMeshes.push(subMesh);
			}

			return true;
		}

		virtual bool LoadResource(Resource* resource, const char* name, U64 size, const U8* data)
		{
			Model* model = reinterpret_cast<Model*>(resource);
			if (!model || size <= 0 || data == nullptr) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

			InputMemoryStream inputStream(data, (U32)size);

			// read shader general header
			ModelGeneralHeader generalHeader;
			if (!inputStream.Read(&generalHeader, sizeof(generalHeader)))
			{
				Logger::Warning("Failed to read model general header");
				return false;
			}

			// check magic and version
			if (generalHeader.mMagic != ModelGeneralHeader::MAGIC ||
				generalHeader.mMajor != ModelGeneralHeader::MAJOR ||
				generalHeader.mMinor != ModelGeneralHeader::MINOR)
			{
				Logger::Warning("Model version mismatch.");
				return false;
			}

			ModelImpl* impl = CJING_NEW(ModelImpl);
			// parse meshes
			if (!ParseMesh(*impl, generalHeader, *model, inputStream))
			{
				Logger::Warning("Failed to parse model");
				CJING_SAFE_DELETE(impl);
				return false;
			}

			model->mImpl = impl;
			Logger::Info("[Resource] Model loaded successful:%s.", name);
			return true;
		}

		virtual bool DestroyResource(Resource* resource)
		{
			if (resource == nullptr) {
				return false;
			}

			Model* model = reinterpret_cast<Model*>(resource);
			CJING_DELETE(model);
			return true;
		}
	};
	DEFINE_RESOURCE(Model, "Model");

	Model::Model()
	{
	}

	Model::~Model()
	{
	}
}