#pragma once

#include "gpu\gpu.h"
#include "resource\resource.h"
#include "resource\resRef.h"

namespace Cjing3D
{
	class Model : public Resource
	{
	public:
		DECLARE_RESOURCE(Model, "Model")

		Model();
		~Model();

	private:
		friend class ModelFactory;

		Model(const Model& rhs) = delete;
		Model& operator=(const Model& rhs) = delete;
	};
	using ModelRef = ResRef<Model>;
}