#pragma once

#include "client\app\mainComponent.h"
#include "imguiRhi\pipeline.h"
#include "renderer\renderPath\renderGraphPath3D.h"

namespace Cjing3D
{
	class GameEditorRenderer : public RenderGraphPath3D
	{
	public:
		void Start()override;
		void Stop()override;
		void UpdatePipelines()override;
		void Compose(GPU::CommandList& cmd)override;

	private:
		ImGuiPipeline mImGuiPipeline;
	};

	class GameEditor : public MainComponent
	{
	public:
		GameEditor(const std::shared_ptr<Engine>& engine);
		virtual ~GameEditor();

		void Initialize()override;
		void Uninitialize()override;

	private:
		GameEditorRenderer* mRenderer = nullptr;
	};

}