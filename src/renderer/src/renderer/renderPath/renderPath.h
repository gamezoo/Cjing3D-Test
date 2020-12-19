#pragma once

#include "gpu\device.h"

namespace Cjing3D
{
	class RenderPath
	{
	public:
		RenderPath() {};
		virtual ~RenderPath() {};

		virtual void Start() {};
		virtual void Stop() {};
		virtual void Update(F32 dt) {};
		virtual void FixedUpdate() {};
		virtual void Render() {};
		virtual void Compose() {};
	};
}