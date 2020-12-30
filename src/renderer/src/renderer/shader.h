#pragma once

#include "resource\resource.h"
#include "resource\resRef.h"

namespace Cjing3D
{
	class Shader : public Resource
	{
	public:
		DECLARE_RESOURCE(Shader, "Shader")

	private:
		friend class ShaderFactory;

	};
	using ShaderRef = ResRef<Shader>;
}