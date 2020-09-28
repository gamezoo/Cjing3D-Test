#include "intersectable.h"

namespace Cjing3D
{
	Frustum::Frustum()
	{
	}

	Frustum::~Frustum()
	{
	}

	void Frustum::SetupFrustum(const F32x4x4& view, const F32x4x4& projection, float screenDepth)
	{
		F32x4x4 temp_proj = projection;

		// Calculate the minimum Z distance in the frustum.
		float zMinimum = -temp_proj.At(4, 3) / temp_proj.At(3, 3);
		float r = screenDepth / (screenDepth - zMinimum);
		temp_proj.At(3, 3) = r;
		temp_proj.At(4, 3) = -r * zMinimum;

		// Create the frustum matrix from the view matrix and updated projection matrix.
		F32x4x4 t = MatrixMultiply(view, temp_proj);

		// Calculate near plane of frustum.
		mleftPlane   = PlaneNormalize(t.GetRow(0) + t.GetRow(3));
		mRightPlane  = PlaneNormalize(t.GetRow(3) - t.GetRow(0));
		mTopPlane    = PlaneNormalize(t.GetRow(3) - t.GetRow(1));
		mBottomPlane = PlaneNormalize(t.GetRow(3) + t.GetRow(1));
		mNearPlane   = PlaneNormalize(t.GetRow(2));
		mFarPlane    = PlaneNormalize(t.GetRow(3) - t.GetRow(2));
	}

	void Frustum::SetupFrustum(const F32x4x4& transform)
	{
		// 根据最终是否在视口范围内创建视锥体，所有的平面法线
		// 方向都是朝内的
		const auto t = MatrixTranspose(transform);
		// p' = p T = (pC1, pC2, pC3, pC4)

		// (-1 < x'/w)  => (-w' <= x)'
		// 0 <= x' + w' => p(C1 + C4)
		mleftPlane   = PlaneNormalize(t.GetRow(0) + t.GetRow(3));
		mRightPlane  = PlaneNormalize(t.GetRow(3) - t.GetRow(0));
		mTopPlane    = PlaneNormalize(t.GetRow(3) - t.GetRow(1));
		mBottomPlane = PlaneNormalize(t.GetRow(3) + t.GetRow(1));
		mNearPlane   = PlaneNormalize(t.GetRow(2));
		mFarPlane    = PlaneNormalize(t.GetRow(3) - t.GetRow(2));
	}

	bool Frustum::Overlaps(const AABB & aabb) const
	{
		// 检测AABB在每个平面的法线方向的最大点与法线的点积
		// 小于0则表示极点在平面外
		for (int i = 0; i < std::size(mPlanes); i++)
		{
			const auto p = aabb.GetMaxPointAlongNormal(mPlanes[i]);
			if (PlaneDotCoord(mPlanes[i], p).x() < 0.0f) {
				return false;
			}
		}
		return true;
	}

	const F32x3 AABB::GetMaxPointAlongNormal(const F32x4& n) const
	{
		// 返回法线法向最大值
		// if n.x > 0 => control.x = 0xFFFFFFF otherwise control.x = 0x0
		const auto control = VectorGreaterOrEqual(n, F32x4::Zero());
		// ret.x = (control.x = 0xffffff)? mMax.x : mMin.x
		return VectorSelect(GetMin(), GetMax(), control);
	}

	AABB AABB::GetByTransforming(const F32x4x4& mat) const
	{
		F32x3 corners[8];
	
		// 变换所有顶点后，取极值做AABB
		MATRIX loadedMat = XMLoad(mat);
		for (int i = 0; i < 8; ++i)
		{
			corners[i] = Vector3Transform(Corner(i), loadedMat);
		}

		F32x3 min = corners[0];
		F32x3 max = corners[1];
		for (int i = 0; i < 8; ++i)
		{
			const F32x3& p = corners[i];
			for (int j = 0; j < 3; j++)
			{
				if (p[j] < min[j]) {
					min[j] = p[j];
				}

				if (p[j] > max[j]) {
					max[j] = p[j];
				}
			}
		}
		return AABB(min, max);
	}

	MATRIX AABB::GetBoxMatrix() const
	{
		F32x3 ext = GetRadius();
		MATRIX sca = XMMatrixScaling(ext[0], ext[1], ext[2]);
		F32x3 pos = GetCenter();
		MATRIX trans = XMMatrixTranslation(pos[0], pos[1], pos[2]);

		return sca * trans;
	}

	void AABB::CopyFromOther(const AABB & aabb)
	{
		mMin = aabb.mMin;
		mMax = aabb.mMax;
	}

	bool AABB::Intersects(const AABB& other) const
	{
		if ((mMax.x() < other.mMin.x() || mMin.x() > other.mMax.x()) ||
			(mMax.y() < other.mMin.y() || mMin.y() > other.mMax.y()) ||
			(mMax.z() < other.mMin.z() || mMin.z() > other.mMax.z())) {
		
			return false;
		}

		return true;
	}

	bool AABB::Intersects(const Ray& other, F32* t) const
	{
		// 射线与轴对称包围盒相交检测
		// 当存在某一方向轴的最小值大于方向轴的最大值时，说明未相交
		if (Intersects(other.mOrigin)) {
			return true;
		}

		F32 maxValue = 0.0f;
		F32 minValue = 0.0f;

		// x axis
		F32 tx1 = (mMin.x() - other.mOrigin.x()) * other.mInvDirection.x();
		F32 tx2 = (mMax.x() - other.mOrigin.x()) * other.mInvDirection.x();
		minValue = std::min(tx1, tx2);
		maxValue = std::max(tx1, tx2);

		// y axis
		F32 ty1 = (mMin.y() - other.mOrigin.y()) * other.mInvDirection.y();
		F32 ty2 = (mMax.y() - other.mOrigin.y()) * other.mInvDirection.y();
		// 找到最大的最小变和最小的最大边
		minValue = std::max(minValue, std::min(ty1, ty2));
		maxValue = std::min(maxValue, std::max(ty1, ty2));

		// z axis
		F32 tz1 = (mMin.z() - other.mOrigin.z()) * other.mInvDirection.z();
		F32 tz2 = (mMax.z() - other.mOrigin.z()) * other.mInvDirection.z();
		// 找到最大的最小变和最小的最大边
		minValue = std::max(minValue, std::min(tz1, tz2));
		maxValue = std::min(maxValue, std::max(tz1, tz2));

		bool isIntersected = maxValue >= minValue;
		if (isIntersected && t != nullptr) {
			*t = minValue;
		}

		return isIntersected;
	}

	bool AABB::Intersects(const F32x3& pos) const
	{
		if (pos.x() < mMin.x() || pos.x() > mMax.x()) return false;
		if (pos.y() < mMin.y() || pos.y() > mMax.y()) return false;
		if (pos.z() < mMin.z() || pos.z() > mMax.z()) return false;
		return true;
	}

	bool Sphere::Intersects(const AABB& other) const
	{
		F32x3 closestPoint = ArrayMin(ArrayMax(other.mMin, mCenter), other.mMax);
		return Distance(mCenter, closestPoint) < mRadius;
	}

	bool Sphere::Intersects(const Sphere& other) const
	{
		return Distance(mCenter, other.mCenter) < (mRadius + other.mRadius);
	}

	bool Sphere::Intersects(const Ray& other) const
	{
		return Vector3LinePointDistance(other.mOrigin, other.mOrigin + other.mDirection, mCenter) <= mRadius;
	}

	bool Ray::Intersects(const Sphere& sphere) const
	{
		return sphere.Intersects(*this);
	}

	bool Ray::Intersects(const AABB& aabb, F32* t) const
	{
		return aabb.Intersects(*this, t);
	}

}