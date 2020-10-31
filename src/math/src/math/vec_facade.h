#pragma once

#include "vec.h"
#include "maths_common.h"

namespace Cjing3D
{
	using VECTOR = XMVECTOR;

	////////////////////////////////////////////////////////////////////////////
	// type checks
	////////////////////////////////////////////////////////////////////////////
	static_assert(sizeof(I32x2) == sizeof(XMFLOAT2), "DirectXMath type mismatch");
	static_assert(sizeof(I32x3) == sizeof(XMFLOAT3), "DirectXMath type mismatch");
	static_assert(sizeof(I32x4) == sizeof(XMFLOAT4), "DirectXMath type mismatch");

	static_assert(sizeof(F32x2) == sizeof(XMFLOAT2), "DirectXMath type mismatch");
	static_assert(sizeof(F32x3) == sizeof(XMFLOAT3), "DirectXMath type mismatch");
	static_assert(sizeof(F32x4) == sizeof(XMFLOAT4), "DirectXMath type mismatch");

	static_assert(sizeof(U32x2) == sizeof(XMUINT2),  "DirectXMath type mismatch");
	static_assert(sizeof(U32x3) == sizeof(XMUINT3),  "DirectXMath type mismatch");
	static_assert(sizeof(U32x4) == sizeof(XMUINT4),  "DirectXMath type mismatch");

	////////////////////////////////////////////////////////////////////////////
	// load
	////////////////////////////////////////////////////////////////////////////

	inline const VECTOR XM_CALLCONV XMLoad(const F32& src) noexcept {
		return XMLoadFloat(&src);
	}

	inline const VECTOR XM_CALLCONV XMLoad(const F32x2& src) noexcept {
		return XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const F32x3& src) noexcept {
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const F32x4& src) noexcept {
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const I32& src) noexcept {
		VECTOR result = XMVectorZero();
		result = XMVectorSetX(result, (F32)src);
		return result;
	}

	inline const VECTOR XM_CALLCONV XMLoad(const I32x2& src) noexcept {
		return XMLoadSInt2(reinterpret_cast<const XMINT2*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const I32x3& src) noexcept {
		return XMLoadSInt3(reinterpret_cast<const XMINT3*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const I32x4& src) noexcept {
		return XMLoadSInt4(reinterpret_cast<const XMINT4*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const U32& src) noexcept {
		VECTOR result = XMVectorZero();
		result = XMVectorSetX(result, (F32)src);
		return result;
	}

	inline const VECTOR XM_CALLCONV XMLoad(const U32x2& src) noexcept {
		return XMLoadUInt2(reinterpret_cast<const XMUINT2*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const U32x3& src) noexcept {
		return XMLoadUInt3(reinterpret_cast<const XMUINT3*>(&src));
	}

	inline const VECTOR XM_CALLCONV XMLoad(const U32x4& src) noexcept {
		return XMLoadUInt4(reinterpret_cast<const XMUINT4*>(&src));
	}

	////////////////////////////////////////////////////////////////////////////
	// store
	////////////////////////////////////////////////////////////////////////////
	template< typename T >
	const T XM_CALLCONV XMStore(FXMVECTOR src) noexcept;

	template<>
	inline const F32 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		F32 dst;
		XMStoreFloat(&dst, src);
		return dst;
	}

	template<>
	inline const F32x2 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		F32x2 dst;
		XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&dst), src);
		return dst;
	}

	template<>
	inline const F32x3 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		F32x3 dst;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&dst), src);
		return dst;
	}

	template<>
	inline const F32x4 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		F32x4 dst;
		XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&dst), src);
		return dst;
	}

	
	template<>
	inline const I32 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		return (I32)XMVectorGetX(src);
	}

	template<>
	inline const I32x2 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		I32x2 dst;
		XMStoreSInt2(reinterpret_cast<XMINT2*>(&dst), src);
		return dst;
	}

	template<>
	inline const I32x3 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		I32x3 dst;
		XMStoreSInt3(reinterpret_cast<XMINT3*>(&dst), src);
		return dst;
	}

	template<>
	inline const I32x4 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		I32x4 dst;
		XMStoreSInt4(reinterpret_cast<XMINT4*>(&dst), src);
		return dst;
	}

	template<>
	inline const U32 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		U32 dst;
		XMStoreInt(&dst, src);
		return dst;
	}

	template<>
	inline const U32x2 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		U32x2 dst;
		XMStoreUInt2(reinterpret_cast<XMUINT2*>(&dst), src);
		return dst;
	}

	template<>
	inline const U32x3 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		U32x3 dst;
		XMStoreUInt3(reinterpret_cast<XMUINT3*>(&dst), src);
		return dst;
	}

	template<>
	inline const U32x4 XM_CALLCONV XMStore(FXMVECTOR src) noexcept {
		U32x4 dst;
		XMStoreUInt4(reinterpret_cast<XMUINT4*>(&dst), src);
		return dst;
	}

	////////////////////////////////////////////////////////////////////////////
	// convert
	////////////////////////////////////////////////////////////////////////////

	inline const XMFLOAT2 XM_CALLCONV XMConvert(const F32x2& src) {
		return XMFLOAT2(src[0], src[1]);
	}

	inline const XMFLOAT3 XM_CALLCONV XMConvert(const F32x3& src) {
		return XMFLOAT3(src[0], src[1], src[2]);
	}

	inline const XMFLOAT4 XM_CALLCONV XMConvert(const F32x4& src) {
		return XMFLOAT4(src[0], src[1], src[2], src[3]);
	}

	////////////////////////////////////////////////////////////////////////////
	// method
	////////////////////////////////////////////////////////////////////////////

	inline F32x3 Normalize(const F32x3& n)
	{
		return XMStore<F32x3>(XMVector3Normalize(XMLoad(n)));
	}

	inline F32 DistanceEstimated(const F32x3& v1, const F32x3& v2)
	{
		XMVECTOR sub = XMLoad(v1 - v2);
		return XMStore<F32>(XMVector3LengthEst(sub));
	}

	inline F32 Distance(const F32x3& v1, const F32x3& v2)
	{
		XMVECTOR sub = XMLoad(v1 - v2);
		return XMStore<F32>(XMVector3Length(sub));
	}

	inline F32x4 PlaneNormalize(const F32x4& v)
	{
		return XMStore<F32x4>(XMPlaneNormalize(XMLoad(v)));
	}

	inline F32x3 PlaneDotCoord(const F32x4& plane, const F32x3& n)
	{
		return XMStore<F32x3>(XMPlaneDotCoord(XMLoad(plane), XMLoad(n)));
	}

	inline F32x4 VectorGreaterOrEqual(const F32x4& v1, const F32x4& v2)
	{
		return XMStore<F32x3>(XMVectorGreaterOrEqual(XMLoad(v1), XMLoad(v2)));
	}

	inline F32x3 VectorSelect(const F32x3& v1, const F32x3& v2, const F32x4& control)
	{
		return XMStore<F32x3>(XMVectorSelect(XMLoad(v1), XMLoad(v2), XMLoad(control)));
	}

	inline F32 Vector3LinePointDistance(const F32x3& linePoint1, const F32x3& linePoint2, const F32x3& point)
	{
		return XMVectorGetX(XMVector3LinePointDistance(XMLoad(linePoint1), XMLoad(linePoint2), XMLoad(point)));
	}
}