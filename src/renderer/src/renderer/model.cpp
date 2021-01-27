#include "model.h"
#include "resource\resourceManager.h"

namespace Cjing3D
{
	class ModelFactory : public ResourceFactory
	{
	public:
		virtual Resource* CreateResource()
		{
			Model* model = CJING_NEW(Model);
			return model;
		}

		virtual bool LoadResource(Resource* resource, const char* name, File& file)
		{
			Model* model = reinterpret_cast<Model*>(resource);
			if (!model || !file) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

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

		virtual bool IsNeedConvert()const
		{
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