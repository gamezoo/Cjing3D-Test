#pragma once

#include "renderPath.h"

namespace Cjing3D
{
	class RenderPath2D : public RenderPath
	{
	public:
		RenderPath2D() {};
		virtual ~RenderPath2D() {};

		void Start()override {};
		void Stop()override {};
		void Update(F32 dt)override {};
		void FixedUpdate()override {};
		void Render()override {};
		 void Compose()override {};
	};
}