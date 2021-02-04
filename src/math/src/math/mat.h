#pragma once

#include "maths_common.h"
#include "vec.h"

namespace Cjing3D
{
	template <size_t R, size_t C, typename T, size_t N = R * C>
	struct Matrix : public std::array<T, N>
	{
		constexpr Matrix()noexcept : std::array<T, N>{} {};
		constexpr Matrix(const T& value)noexcept :
			std::array<T, N>(FillArray<T, N>(value)) {}

		template<typename... Args, typename = std::enable_if_t< (N == sizeof...(Args))> >
		constexpr Matrix(Args&&... args) noexcept :
			std::array<T, N>{ std::forward<Args>(args)...} {}

		T& At(size_t r, size_t c)
		{
			return this->_Elems[r * c];
		}

		const T& At(size_t r, size_t c)const
		{
			return this->_Elems[r * c];
		}

		Array<R, T> GetRow(size_t index) const
		{
			return Array<R, T>(&this->_Elems[index * C]);
		}

		Array<C, T> GetColumn(size_t index) const
		{
			Array<C, T> ret;
			for (size_t c = 0; c < R; c++) {
				ret[c] = At(c, index);
			}
			return ret;
		}
	};

	template <size_t R, size_t C, typename T>
	inline bool operator==(const Matrix<R, C, T>& lhs, const Matrix<R, C, T>& rhs)
	{
		for (size_t i = 0; i < R * C; i++) {
			if (lhs[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	template <size_t R, size_t C, typename T>
	inline bool operator!=(const Matrix<R, C, T>& lhs, const Matrix<R, C, T>& rhs)
	{
		return !(lhs == rhs);
	}

	using F32x3x3 = Matrix<3, 3, F32>;
	using F32x4x4 = Matrix<4, 4, F32>;

	static const F32x4x4 IDENTITY_MATRIX = F32x4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}