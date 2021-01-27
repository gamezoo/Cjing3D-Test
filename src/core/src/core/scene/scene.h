#pragma once

#include "core\common\definitions.h"

namespace Cjing3D
{
namespace ECS
{	
	class IScene
	{
	public:
		virtual~IScene() {}

		virtual void Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void Update(F32 dt) = 0;
		virtual void FixedUpdate() = 0;
		virtual void Clear() = 0;
	};
}
}