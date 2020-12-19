#pragma once

#include "renderPath2D.h"

namespace Cjing3D
{
	class RenderPath3D : public RenderPath2D
	{
	public:
		RenderPath3D() {};
		virtual ~RenderPath3D() {};

		void Start()override {};
		void Stop()override {};
		void Update(F32 dt)override {};
		void FixedUpdate()override {};
		void Render()override {};
		void Compose()override {};
	};
}