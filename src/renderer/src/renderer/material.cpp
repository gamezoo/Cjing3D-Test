#include "material.h"
#include "resource\resourceManager.h"

namespace Cjing3D
{
	class MaterialFactory : public ResourceFactory
	{
	public:
		virtual Resource* CreateResource()
		{
			Material* material = CJING_NEW(Material);
			return material;
		}

		virtual bool LoadResource(Resource* resource, const char* name, File& file)
		{
			Material* material = reinterpret_cast<Material*>(resource);
			if (!material || !file) {
				return false;
			}

			if (!GPU::IsInitialized()) {
				return false;
			}

			Logger::Info("[Resource] Material loaded successful:%s.", name);
			return true;
		}

		virtual bool DestroyResource(Resource* resource)
		{
			if (resource == nullptr) {
				return false;
			}

			Material* material = reinterpret_cast<Material*>(resource);
			CJING_DELETE(material);
			return true;
		}

		virtual bool IsNeedConvert()const
		{
			return true;
		}
	};
	DEFINE_RESOURCE(Material, "Material");

	Material::Material()
	{
	}

	Material::~Material()
	{
	}


}
