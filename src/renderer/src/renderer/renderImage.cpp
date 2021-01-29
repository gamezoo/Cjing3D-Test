#include "renderImage.h"
#include "renderer.h"

namespace Cjing3D
{
	bool mIsInitialized = false;

	/// ///////////////////////////////////////////////////////////////////
	// Fullscreen pipeline
	void RenderImage::Initialize()
	{
		mIsInitialized = true;
	}

	void RenderImage::Draw(const GPU::ResHandle& tex, const ImageParams& param, GPU::CommandList& cmd)
	{
		if (!mIsInitialized) {
			return;
		}

		auto shader = Renderer::GetShader(SHADERTYPE_IMAGE);
		if (!shader) {
			return;
		}
		cmd.Event("Image");

		// bind resources
		ShaderBindingContext bindingContext(cmd);
		ShaderBindingSet bindingSet = shader->CreateBindingSet("ImageBindingSet");
		bindingSet.Set("gImageTextureBase", GPU::Binding::Texture(tex, GPU::SHADERSTAGES_PS));

		// fullscreen
		if (param.IsFullScreen())
		{
			ShaderTechniqueDesc desc = {};
			desc.mPrimitiveTopology = GPU::TRIANGLESTRIP;
			auto tech = shader->CreateTechnique("TECH_FULLSCREEN", desc);

			if (bindingContext.Bind(tech, bindingSet))
			{
				auto pipelineState = tech.GetPipelineState();
				cmd.BindPipelineState(pipelineState);
				cmd.Draw(3, 0);
			}
		}
	}
}