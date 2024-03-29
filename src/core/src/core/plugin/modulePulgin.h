#pragma once

#include "plugin.h"
#include "core\scene\universe.h"

namespace Cjing3D
{
	class ModulerPlugin : public Plugin
	{
	public:
		PULGIN_DECLARE(Moduler)

		virtual~ModulerPlugin() {}

		virtual void Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void Update(F32 dt) = 0;
		virtual void CreateScene(Universe& universe) = 0;
	};
}