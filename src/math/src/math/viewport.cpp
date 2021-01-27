#include "viewport.h"

namespace Cjing3D
{
	void Viewport::CreatePerspective(F32 width, F32 height, F32 near, F32 far, F32 fov)
	{
		mWidth = width;
		mHeight = height;
		mNear = near;
		mFar = far;
		mFov = fov;
	}

	void Viewport::Update()
	{
		// view
		MATRIX view = XMMatrixLookToLH(
			XMLoad(mEye),
			XMLoad(mAt),
			XMLoad(mUp)
		);
		mView = XMStore<F32x4x4>(view);

		// projection
		MATRIX projection = XMMatrixPerspectiveFovLH(mFov, mWidth / mHeight, mFar, mNear);
		mProjection = XMStore<F32x4x4>(projection);

		// viewProjection
		MATRIX vp = XMMatrixMultiply(view, projection);
		mViewProjection = XMStore<F32x4x4>(vp);

		// inverse
		mInvView = XMStore<F32x4x4>(XMMatrixInverse(nullptr, view));
		mInvProjection = XMStore<F32x4x4>(XMMatrixInverse(nullptr, projection));
		mInvViewProjection = XMStore<F32x4x4>(XMMatrixInverse(nullptr, vp));

		// frustum
		mFrustum.SetupFrustum(mViewProjection);
	}
}