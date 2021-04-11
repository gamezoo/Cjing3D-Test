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

			// parse meshes

			// parse bones


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