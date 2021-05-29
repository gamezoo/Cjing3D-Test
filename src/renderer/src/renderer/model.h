#pragma once

#include "gpu\gpu.h"
#include "resource\resource.h"
#include "resource\resRef.h"

namespace Cjing3D
{
	struct ModelSubMesh
	{
		I32 mIndexOffset = 0;
		I32 mIndices = 0;
	};

	struct ModelMesh
	{
		DynamicArray<ModelSubMesh> mSubMeshes;
	};

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

		struct ModelImpl* mImpl = nullptr;
	};
	using ModelRef = ResRef<Model>;
}