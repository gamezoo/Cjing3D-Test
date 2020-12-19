#pragma once

#include "renderPath.h"

namespace Cjing3D
{
	class ScriptablePath : public RenderPath
	{
	public:
		ScriptablePath() {};
		virtual ~ScriptablePath() {};

		void Start()override {};
		void Stop()override {};
		void Update(F32 dt)override {};
		void FixedUpdate()override {};
		void Render()override {};
		void Compose()override {};
	};
}