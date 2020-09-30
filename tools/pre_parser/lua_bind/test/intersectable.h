#pragma once

#include "maths_common.h"
#include "vec_facade.h"
#include "mat_facade.h"
#include "geometry.h"

namespace Cjing3D
{
	class AABB;
	class Ray;

	class [[LUA_BINDER_REGIST]] [[LUA_BINDER_NAME(LuaSphere)]] Sphere final
	{
	public:
		F32x3 mCenter;
		F32   mRadius;

		[[LUA_BINDER_REGIST]]
		Sphere() :mCenter(), mRadius(0.0f) {}

		[[LUA_BINDER_REGIST]]
		Sphere(F32x3 center, F32 radius) : mCenter(center), mRadius(radius) {}

		bool Intersects(const AABB& other)const;
		bool Intersects(const Sphere& other)const;
		bool Intersects(const Ray& other)const;
	};

	class [[LUA_BINDER_REGIST]] AABB final
	{
	public:
		F32x3 mMin;
		F32x3 mMax;

	public:
		explicit AABB() : AABB(F32x3(FLT_MAX, FLT_MAX, FLT_MAX), F32x3(-FLT_MAX, -FLT_MAX, -FLT_MAX)) {};
		explicit AABB(F32x3 p) : mMin(p), mMax(p) {};
		explicit AABB(F32x3 min, F32x3 max) : mMin(min), mMax(max) {};
		AABB(const AABB& aabb) = default;
		AABB(AABB&& aabb) = default;
		~AABB() = default;

		AABB& operator= (const AABB& aabb) = default;
		AABB& operator= (AABB&& aabb) = default;

		[[LUA_BINDER_REGIST]]
		static AABB CreateFromHalfWidth(F32x3 position, F32x3 range)
		{
			F32x3 minVec = position - range;
			F32x3 maxVec = position + range;
			return AABB(minVec, maxVec);
		}

		[[LUA_BINDER_REGIST]]
		const F32x3 GetMin()const {
			return mMin;
		}

		[[LUA_BINDER_REGIST]]
		const F32x3 GetMax()const {
			return mMax;
		}

		const F32x3 GetCenter()const {

			return (GetMin() + GetMax()) * 0.5f;
		}

		const F32x3 GetDiagonal()const {
			return (GetMax() - GetMin());
		}

		const F32x3 GetRadius()const {
			return GetDiagonal() * 0.5f;
		}

		template< typename VertexT>
		static const AABB Union(const AABB& aabb, const VertexT& vertex)
		{
			return Union(aabb, vertex.mPosition);
		}

		static const AABB Union(const AABB& aabb, const F32x3& point)
		{
			return Union(aabb, AABB(point));
		}

		static const AABB Union(const AABB& aabb1, const AABB& aabb2)
		{
			auto pMin = ArrayMin(aabb1.GetMin(), aabb2.GetMin());
			auto pMax = ArrayMax(aabb1.GetMax(), aabb2.GetMax());
			return AABB(pMin, pMax);
		}

		const F32x3 GetMaxPointAlongNormal(const F32x4& n)const;
		AABB GetByTransforming(const F32x4x4& mat)const;
		MATRIX GetBoxMatrix()const;
		void CopyFromOther(const AABB& aabb);

		bool Intersects(const AABB& other)const;
		bool Intersects(const Ray& other, F32* t = nullptr)const;
		bool Intersects(const F32x3& pos)const;

		inline F32x3 Corner(int index) const
		{
			auto _min = GetMin();
			auto _max = GetMax();
			switch (index)
			{
			case 0: return _min;
			case 1: return F32x3(_min[0], _max[1], _min[2]);
			case 2: return F32x3(_min[0], _max[1], _max[2]);
			case 3: return F32x3(_min[0], _min[1], _max[2]);
			case 4: return F32x3(_max[0], _min[1], _min[2]);
			case 5: return F32x3(_max[0], _max[1], _min[2]);
			case 6: return _max;
			case 7: return F32x3(_max[0], _min[1], _max[2]);
			}
			assert(0);
			return F32x3();
		}

		inline void SetFromHalfWidth(const F32x3& center, const F32x3& range)
		{
			mMin = F32x3(center.x() - range.x(), center.y() - range.y(), center.z() - range.z());
			mMax = F32x3(center.x() + range.x(), center.y() + range.y(), center.z() + range.z());
		}
	};
	
	class Frustum
	{
	public:
		Frustum();
		~Frustum();

		void SetupFrustum(const F32x4x4& view, const F32x4x4& projection, float screenDepth);
		void SetupFrustum(const F32x4x4& transform);
		bool Overlaps(const AABB& aabb)const;

	private:
		union {
			struct {
				F32x4 mleftPlane;
				F32x4 mRightPlane;
				F32x4 mTopPlane;
				F32x4 mBottomPlane;
				F32x4 mFarPlane;
				F32x4 mNearPlane;
			};
			F32x4 mPlanes[6];
		};
	};

	// Direct::BoundingFrustum
	inline DirectX::BoundingFrustum CreateBoundingFrustum(const XMMATRIX& projection)
	{
		BoundingFrustum newFrustum;
		BoundingFrustum::CreateFromMatrix(newFrustum, projection);
		std::swap(newFrustum.Near, newFrustum.Far);
		return newFrustum;
	}

	inline void TransformBoundingFrustum(DirectX::BoundingFrustum& frustum, const XMMATRIX& transform)
	{
		frustum.Transform(frustum, transform);
		XMStoreFloat4(&frustum.Orientation, XMQuaternionNormalize(XMLoadFloat4(&frustum.Orientation)));
	}

	class Ray
	{
	public:
		F32x3 mOrigin = F32x3(0.0f, 0.0f, 0.0f);
		F32x3 mDirection = F32x3(0.0f, 0.0f, 1.0f);
		F32x3 mInvDirection = F32x3(0.0f, 0.0f, 1.0f);

		Ray() = default;
		Ray(const F32x3& origin, const F32x3& direction) :
			mOrigin(origin), 
			mDirection(direction),
			mInvDirection(F32x3(1.0f) / mDirection)
		{
		}

		bool Intersects(const Sphere& sphere)const;
		bool Intersects(const AABB& aabb, F32* t = nullptr)const;
	};
}