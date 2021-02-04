#pragma once

#include "mat_facade.h"

namespace Cjing3D
{
	class Transform
	{
	public:
		F32x3 mTranslationLocal = F32x3(0.0f, 0.0f, 0.0f);
		F32x4 mRotationLocal = F32x4(0.0f, 0.0f, 0.0f, 1.0f);
		F32x3 mScaleLocal = F32x3(1.0f, 1.0f, 1.0f);
		F32x4x4 mWorld = IDENTITY_MATRIX;
		bool mIsDirty = true;

	public:
		Transform();
		
		void Update();	
		void Clear();
		
		void Translate(const F32x3& value);
		void RotateRollPitchYaw(const F32x3& value);
		void Rotate(const F32x4& quaternion);
		void Scale(const F32x3& value);

		inline F32x4 GetRotationLocal()const { return mRotationLocal; }
		inline F32x3 GetTranslationLocal()const { return mTranslationLocal; }
		inline F32x3 GetScaleLocal()const { return mScaleLocal; }

		F32x3 GetPosition()const;
		F32x4 GetRotation()const;
		F32x3 GetScale()const;

		MATRIX GetLocalMatrix()const;
		void UpdateByMatrix(const MATRIX& matrix);
		void WorldMatrixToLocal();
		void UpdateFromParent(const Transform& parent);

		void SetDirty(bool isDirty) { mIsDirty = isDirty; }
		bool IsDirty()const { return mIsDirty; }
		inline F32x4x4 GetWorldTransform() const { return mWorld; }
		inline MATRIX GetWorldMatrix()const { return XMLoad(mWorld); }
		inline MATRIX GetInvertedWorldMatrix()const { return XMMatrixInverse(nullptr, XMLoad(mWorld)); }

		static const Transform IDENTITY;
	};
}