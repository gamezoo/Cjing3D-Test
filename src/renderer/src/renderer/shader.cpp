#include "shader.h"
#include "resource\resourceManager.h"

namespace Cjing3D
{
	class ShaderFactory : public ResourceFactory
	{
	public:
		virtual Resource* CreateResource()
		{
			Shader* shader = CJING_NEW(Shader);
			return shader;
		}

		virtual bool LoadResource(Resource* resource, const char* name, File& file)
		{
			if (!file) {
				return false;
			}

			return true;
		}

		virtual bool DestroyResource(Resource* resource)
		{
			if (resource == nullptr) {
				return false;
			}

			Shader* shader = reinterpret_cast<Shader*>(resource);
			CJING_DELETE(shader);
			return true;
		}

		virtual bool IsNeedConvert()const
		{
			return true;
		}
	};

	DEFINE_RESOURCE(Shader, "Shader");
}