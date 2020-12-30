#pragma once

#include "core\common\definitions.h"

namespace Cjing3D
{
	class IScene
	{
	public:
		virtual~IScene() {}

		virtual void Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void Update(F32 dt) = 0;
	};
}