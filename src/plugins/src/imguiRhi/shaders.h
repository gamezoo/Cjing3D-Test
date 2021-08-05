#pragma once

#include "gpu\gpu.h"

namespace Cjing3D
{
namespace ImGuiRHI
{
	class ImguiShaderCompiler;

	class ShaderFactory
	{
	public:
		ShaderFactory();
		~ShaderFactory();

		GPU::ResHandle CreateVertexShader();
		GPU::ResHandle CreatePixelShader();

	private:
		ImguiShaderCompiler* mCompiler = nullptr;
	};
}
}