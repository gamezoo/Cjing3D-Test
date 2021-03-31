#pragma once

#include "gpu\gpu.h"
#include "resource\resource.h"
#include "resource\resRef.h"
#include "shader.h"
#include "texture.h"

namespace Cjing3D
{
	class MaterialImpl;

	class Material : public Resource
	{
	public:
		DECLARE_RESOURCE(Material, "Material")

		enum ShaderType
		{
			ShaderType_Standard,
			ShaderType_Custom,
		};

		enum TextureSlot
		{
			BaseColorMap,
			NormalMap,
			SurfaceMap,

			TextureSlot_Count,
		};

		Material();
		~Material();

		bool IsValid()const;

		ShaderTechnique CreateTechnique(const ShaderTechHasher& hasher, const ShaderTechniqueDesc& desc);
		ShaderTechnique CreateTechnique(const char* name, const ShaderTechniqueDesc& desc);

		ShaderRef GetShader();
		GPU::ResHandle GetTexture(TextureSlot slot)const;

	private:
		friend class ModelFactory;

		Material(const Material& rhs) = delete;
		Material& operator=(const Material& rhs) = delete;

		MaterialImpl* mImpl = nullptr;
	};
	using MaterialRef = ResRef<Material>;
}