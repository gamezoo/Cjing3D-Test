#pragma once

#include "maths_common.h"
#include "mat.h"
#include "vec_facade.h"

namespace Cjing3D
{
	using MATRIX = XMMATRIX;

	////////////////////////////////////////////////////////////////////////////
	// load
	////////////////////////////////////////////////////////////////////////////
	inline const MATRIX XM_CALLCONV XMLoad(const F32x3x3& src) noexcept {
		return XMLoadFloat3x3(reinterpret_cast<const XMFLOAT3X3*>(&src));
	}

	inline const MATRIX XM_CALLCONV XMLoad(const F32x4x4& src) noexcept {
		return XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&src));
	}

	////////////////////////////////////////////////////////////////////////////
	// store
	////////////////////////////////////////////////////////////////////////////
	template< typename T >
	const T XM_CALLCONV XMStore(MATRIX src) noexcept;

	template<>
	inline const F32x3x3 XM_CALLCONV XMStore(MATRIX src) noexcept {
		F32x3x3 dst;
		XMStoreFloat3x3(reinterpret_cast<XMFLOAT3X3*>(&dst), src);
		return dst;
	}

	template<>
	inline const F32x4x4 XM_CALLCONV XMStore(MATRIX src) noexcept {
		F32x4x4 dst;
		XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&dst), src);
		return dst;
	}

	////////////////////////////////////////////////////////////////////////////
	// method
	////////////////////////////////////////////////////////////////////////////

	inline F32x4x4 MatrixMultiply(const F32x4x4& lhs, const F32x4x4& rhs)
	{
		return XMStore<F32x4x4>(XMMatrixMultiply(XMLoad(lhs), XMLoad(rhs)));
	}

	inline F32x4x4 MatrixTranspose(const F32x4x4& mat)
	{
		return XMStore<F32x4x4>(XMMatrixTranspose(XMLoad(mat)));
	}

	inline F32x3 Vector3Transform(const F32x3& v1, const XMMATRIX& mat)
	{
		return XMStore<F32x3>(XMVector3Transform(XMLoad(v1), mat));
	}
}