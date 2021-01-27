#pragma once

#include "gpu\gpu.h"
#include "resource\resource.h"
#include "resource\resRef.h"

namespace Cjing3D
{
	class Material : public Resource
	{
	public:
		DECLARE_RESOURCE(Material, "Material")

		Material();
		~Material();

	private:
		friend class ModelFactory;

		Material(const Material& rhs) = delete;
		Material& operator=(const Material& rhs) = delete;
	};
	using MaterialRef = ResRef<Material>;
}