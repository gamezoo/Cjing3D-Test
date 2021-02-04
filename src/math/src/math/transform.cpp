#include "transform.h"

namespace Cjing3D
{
	const Transform Transform::IDENTITY;

	Transform::Transform()
	{
	}

	void Transform::Update()
	{
		if (IsDirty()) 
		{
			SetDirty(false);
			mWorld = XMStore<F32x4x4>(GetLocalMatrix());
		}
	}

	void Transform::Clear()
	{
		mTranslationLocal = F32x3(0.0f, 0.0f, 0.0f);
		mRotationLocal = F32x4(0.0f, 0.0f, 0.0f, 1.0f);
		mScaleLocal = F32x3(1.0f, 1.0f, 1.0f);
		SetDirty(true);
	}

	void Transform::Translate(const F32x3& value)
	{
		mTranslationLocal += value;
		SetDirty(true);
	}

	void Transform::RotateRollPitchYaw(const F32x3& value)
	{
		VECTOR quat = XMLoad(mRotationLocal);
		VECTOR x = XMQuaternionRotationRollPitchYaw(value.x(), 0.0f, 0.0f);
		VECTOR y = XMQuaternionRotationRollPitchYaw(0.0f, value.y(), 0.0f);
		VECTOR z = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, value.z());

		quat = XMQuaternionMultiply(x, quat);
		quat = XMQuaternionMultiply(quat, y);
		quat = XMQuaternionMultiply(z, quat);
		quat = XMQuaternionNormalize(quat);

		mRotationLocal = XMStore<F32x4>(quat);
		SetDirty(true);
	}

	void Transform::Scale(const F32x3& value)
	{
		mScaleLocal *= value;
		SetDirty(true);
	}

	void Transform::Rotate(const F32x4& quaternion)
	{
		VECTOR quat = XMQuaternionMultiply(XMLoad(mRotationLocal), XMLoad(quaternion));
		quat = XMQuaternionNormalize(quat);
		mRotationLocal = XMStore<F32x4>(quat);
		SetDirty(true);
	}

	F32x3 Transform::GetPosition() const
	{
		return mWorld.GetRow(3).xyz();
	}

	F32x4 Transform::GetRotation() const
	{
		VECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoad(mWorld));
		return XMStore<F32x4>(R);
	}

	F32x3 Transform::GetScale() const
	{
		VECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoad(mWorld));
		return XMStore<F32x3>(S);
	}

	MATRIX Transform::GetLocalMatrix() const
	{
		VECTOR s = XMLoad(mScaleLocal);
		VECTOR r = XMLoad(mRotationLocal);
		VECTOR t = XMLoad(mTranslationLocal);
		return XMMatrixScalingFromVector(s) * XMMatrixRotationQuaternion(r) * XMMatrixTranslationFromVector(t);
	}

	void Transform::UpdateByMatrix(const MATRIX& matrix)
	{
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, GetLocalMatrix() * matrix);
		mScaleLocal = XMStore<F32x3>(S);
		mRotationLocal = XMStore<F32x4>(R);
		mTranslationLocal = XMStore<F32x3>(T);

		SetDirty(true);
	}

	void Transform::WorldMatrixToLocal()
	{
		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoad(mWorld));
		mScaleLocal = XMStore<F32x3>(S);
		mRotationLocal = XMStore<F32x4>(R);
		mTranslationLocal = XMStore<F32x3>(T);

		SetDirty(true);
	}

	void Transform::UpdateFromParent(const Transform& parent)
	{
		MATRIX newWorld = GetLocalMatrix() * XMLoad(parent.mWorld);
		mWorld = XMStore<F32x4x4>(newWorld);
	}
}