#pragma once

#include "intersectable.h"
#include "geometry.h"

namespace Cjing3D
{
	class Viewport
	{
	public:
		F32 mWidth = 0.0f;
		F32 mHeight = 0.0f;
		F32 mNear = 0.1f;
		F32 mFar = 800.0f;
		F32 mFov = XM_PI / 3.0f;

		F32x3 mEye = F32x3(0.0f, 0.0f, 0.0f);
		F32x3 mAt = F32x3(0.0f, 0.0f, 1.0f);
		F32x3 mUp = F32x3(0.0f, 1.0f, 0.0f);

		F32x4x4 mView, mProjection, mViewProjection;
		F32x4x4 mInvView, mInvProjection, mInvViewProjection;
		Frustum mFrustum;

	public:
		void CreatePerspective(F32 width, F32 height, F32 near, F32 far, F32 fov = XM_PI / 3.0f);
		void Update();

		MATRIX GetViewMatrix()const { return XMLoad(mView); };
		MATRIX GetProjectionMatrix()const { return XMLoad(mProjection); };
		MATRIX GetViewProjectionMatrix()const { return XMLoad(mViewProjection); };
		MATRIX GetInvViewMatrix()const { return XMLoad(mInvView); }
		MATRIX GetInvProjectionMatrix()const { return XMLoad(mInvProjection); }
		MATRIX GetInvViewProjectionMatrix()const { return XMLoad(mInvViewProjection); }
	};
}