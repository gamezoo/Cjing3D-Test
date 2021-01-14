#pragma once

#include "resource\resource.h"
#include "resource\resRef.h"

namespace Cjing3D
{
	class ShaderTechnique
	{
	public:

	};

	class Shader : public Resource
	{
	public:
		DECLARE_RESOURCE(Shader, "Shader")
		~Shader();

		ShaderTechnique CreateTechnique(const char* name);

	private:
		friend class ShaderFactory;

		Shader();
		Shader(const Shader& rhs) = delete;
		Shader& operator=(const Shader& rhs) = delete;

		struct ShaderImpl* mImpl = nullptr;
	};
	using ShaderRef = ResRef<Shader>;
}