#pragma once

#include "client\app\mainComponent.h"

namespace Cjing3D
{
	class GameEditor : public MainComponent
	{
	public:
		GameEditor(const std::shared_ptr<Engine>& engine);
		virtual ~GameEditor();
	};

}