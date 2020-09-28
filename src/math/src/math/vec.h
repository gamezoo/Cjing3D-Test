#pragma once

#include "maths_common.h"

#include <array>
#include <tuple>
#include <utility>

namespace Cjing3D
{
	namespace ArrayImpl {

		template<typename T, size_t... I>
		constexpr const auto FillArray(const T& value, std::index_sequence<I...>)
		{
			return std::array<T, sizeof...(I)>{
				(static_cast<void>(I), value)...
			};
		}

		template<typename T, size_t ToN, size_t... I>
		constexpr const auto Enlarge(const std::array<T, sizeof...(I)>& array, std::index_sequence<I...>)
		{
			return std::array<T, ToN>{ array[I]... };
		}

		template<typename T, size_t... I>
		constexpr const auto ArrayToTuple(const std::array<T, sizeof...(I)>& array, std::index_sequence<I...>)
		{
			return std::make_tuple(array[I]...);
		}

		template<typename T, typename TupleT, size_t...I>
		constexpr const auto TupleToArray(const TupleT& tuple, std::index_sequence<I...>)
		{
			return std::array<T, sizeof...(I)>{ std::get<I>(tuple)... };
		}
	}

	template< typename T, size_t N>
	constexpr const auto FillArray(const T& value)
	{
		return ArrayImpl::FillArray(value, std::make_index_sequence<N>());
	}

	template <typename T, size_t ToN, size_t FromN>
	constexpr const auto EnlargeArray(const std::array<T, FromN>& array)
	{
		return ArrayImpl::Enlarge<T, ToN>(array, std::make_index_sequence<FromN>());
	}

	template <typename T, size_t N>
	constexpr const auto ArrayToTuple(const std::array<T, N>& array)
	{
		return ArrayImpl::ArrayToTuple(array, std::make_index_sequence<N>());
	}

	template<typename T, typename... Ts>
	constexpr const auto TupleToArray(const std::tuple<T, Ts...>& t)
	{
		constexpr auto N = sizeof...(Ts) + 1;
		return ArrayImpl::TupleToArray<T>(t, std::make_index_sequence<N>());
	}

	template<size_t N, typename T, size_t AlignT = alignof(T), typename = std::enable_if_t< (N >= 1) >>
	struct alignas(AlignT) Array : public std::array<T, N>
	{
	public:
		constexpr Array()noexcept : std::array<T, N>{} {};
		constexpr Array(const T& value)noexcept :
			std::array<T, N>(FillArray<T, N>(value)) {}

		template<typename... Args, typename = std::enable_if_t< (N == sizeof...(Args))> >
		constexpr Array(Args&&... args) noexcept :
			std::array<T, N>{ std::forward<Args>(args)...} {}

		template<size_t	FromN, typename = std::enable_if_t< (FromN < N)> >
		constexpr Array(const Array<FromN, T, AlignT>& other) noexcept :
			std::array<T, N>(EnlargeArray<T, N>(other)) {}

		template< size_t FromN, typename... Args, typename = std::enable_if_t< (FromN < N && (FromN + sizeof...(Args) == N)) > >
		constexpr Array(const Array<FromN, T, AlignT>& other, Args&&... args) noexcept :
			std::array<T, N>(TupleToArray(std::tuple_cat(ArrayToTuple(other),
				std::make_tuple(std::forward<Args>(args)...)))) {}
	};

	
	template<typename T, size_t AlignT>
	struct alignas(AlignT) Array<2, T, AlignT> : public std::array<T, 2>
	{
		constexpr Array()noexcept : std::array<T, 2>{} {};

		constexpr Array<2, T, AlignT>(const T& v1, const T& v2)
		{
			this->_Elems[0] = v1;
			this->_Elems[1] = v2;
		}

		template<typename S>
		constexpr Array<2, T, AlignT>(const S* src)
		{
			for (int i = 0; i < 2; i++) {
				this->_Elems[i] = (T)src[i];
			}
		}

		constexpr Array<2, T, AlignT>(const T& value)noexcept :
			std::array<T, 2>(FillArray<T, 2>(value)) {}

		template<typename... Args, typename = std::enable_if_t< (2 == sizeof...(Args))> >
		constexpr Array<2, T, AlignT>(Args&&... args) noexcept :
			std::array<T, 2>{ std::forward<Args>(args)...} {}

		inline static Array<2, T, AlignT> One()
		{
			return Array<2, T, AlignT>((T)1, (T)1);
		}
		inline static Array<2, T, AlignT> Zero()
		{
			return Array<2, T, AlignT>((T)0, (T)0);
		}

		inline T x()const { return this->at(0); }
		inline T y()const { return this->at(1); }
	};

	template<typename T, size_t AlignT>
	struct alignas(AlignT) Array<3, T, AlignT> : public std::array<T, 3>
	{
		constexpr Array()noexcept : std::array<T, 3>{} {};

		constexpr Array<3, T, AlignT>(const T& v1, const T& v2, const T& v3)
		{
			this->_Elems[0] = v1;
			this->_Elems[1] = v2;
			this->_Elems[2] = v3;
		}

		template<typename S>
		constexpr Array<3, T, AlignT>(const S* src)
		{
			for (int i = 0; i < 3; i++) {
				this->_Elems[i] = (T)src[i];
			}
		}

		constexpr Array<3, T, AlignT>(const T& value)noexcept :
			std::array<T, 3>(FillArray<T, 3>(value)) {}

		template<typename... Args, typename = std::enable_if_t< (3 == sizeof...(Args))> >
		constexpr Array<3, T, AlignT>(Args&&... args) noexcept :
			std::array<T, 3>{ std::forward<Args>(args)...} {}

		constexpr Array<3, T, AlignT>(const Array<2, T, AlignT>& other) noexcept :
			std::array<T, 3>(EnlargeArray<T, 3>(other)) {}

		template< size_t FromN, typename... Args, typename = std::enable_if_t< (FromN < 3 && (FromN + sizeof...(Args) == 3)) > >
		constexpr Array(const Array<FromN, T, AlignT>& other, Args&&... args) noexcept :
			std::array<T, 3>(TupleToArray(std::tuple_cat(ArrayToTuple(other), std::make_tuple(std::forward<Args>(args)...)))) {}

		inline static Array<3, T, AlignT> One()
		{
			return Array<3, T, AlignT>((T)1, (T)1, (T)1);
		}
		inline static Array<3, T, AlignT> Zero()
		{
			return Array<3, T, AlignT>((T)0, (T)0, (T)0);
		}

		inline T x()const { return this->at(0); }
		inline T y()const { return this->at(1); }
		inline T z()const { return this->at(2); }
	};

	template<typename T, size_t AlignT>
	struct alignas(AlignT) Array<4, T, AlignT> : public std::array<T, 4>
	{
		constexpr Array()noexcept : std::array<T, 4>{} {};

		constexpr Array<4, T, AlignT>(const T& v1, const T& v2, const T& v3, const T& v4)
		{
			this->_Elems[0] = v1;
			this->_Elems[1] = v2;
			this->_Elems[2] = v3;
			this->_Elems[3] = v4;
		}

		template<typename S>
		constexpr Array<4, T, AlignT>(const S* src)
		{
			for (int i = 0; i < 4; i++) {
				this->_Elems[i] = (T)src[i];
			}
		}

		constexpr Array<4, T, AlignT>(const T& value)noexcept :
			std::array<T, 4>(FillArray<T, 4>(value)) {}

		template<typename... Args, typename = std::enable_if_t< (4 == sizeof...(Args))> >
		constexpr Array<4, T, AlignT>(Args&&... args) noexcept :
			std::array<T, 4>{ std::forward<Args>(args)...} {}

		template<size_t	FromN, typename = std::enable_if_t< (FromN < 4)> >
		constexpr Array(const Array<FromN, T, AlignT>& other) noexcept :
			std::array<T, 4>(EnlargeArray<T, 4>(other)) {}

		template< size_t FromN, typename... Args, typename = std::enable_if_t< (FromN < 4 && (FromN + sizeof...(Args) == 4)) > >
		constexpr Array(const Array<FromN, T, AlignT>& other, Args&&... args) noexcept :
			std::array<T, 4>(TupleToArray(std::tuple_cat(ArrayToTuple(other),
				std::make_tuple(std::forward<Args>(args)...)))) {}

		inline static Array<4, T, AlignT> One()
		{
			return Array<4, T, AlignT>((T)1, (T)1, (T)1, (T)1);
		}
		inline static Array<4, T, AlignT> Zero()
		{
			return Array<4, T, AlignT>((T)0, (T)0, (T)0, (T)0);
		}

		inline T x()const { return this->at(0); }
		inline T y()const { return this->at(1); }
		inline T z()const { return this->at(2); }
		inline T w()const { return this->at(3); }
	};

	////////////////////////////////////////////////////////////////////////////
	// operators
	////////////////////////////////////////////////////////////////////////////
	template <size_t N, typename T>
	inline Array<N, T>& operator+=(Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] += rhs[i];
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator+=(Array<N, T>& lhs, const T& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] += rhs;
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator+(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		Array<N, T> ret(lhs);
		ret += rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator+(const Array<N, T>& lhs, const T& rhs)
	{
		Array<N, T> ret(lhs);
		ret += rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator-=(Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] -= rhs[i];
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator-=(Array<N, T>& lhs, const T& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] -= rhs;
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator-(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		Array<N, T> ret(lhs);
		ret -= rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator-(const Array<N, T>& lhs, const T& rhs)
	{
		Array<N, T> ret(lhs);
		ret -= rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator-(const Array<N, T>& rhs)
	{
		Array<N, T> ret;
		for (size_t i = 0; i < N; i++) {
			ret[i] = -rhs[i];
		}
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator*=(Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] *= rhs[i];
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator*=(Array<N, T>& lhs, const T& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] *= rhs;
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator*(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		Array<N, T> ret(lhs);
		ret *= rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator*(const Array<N, T>& lhs, const T& rhs)
	{
		Array<N, T> ret(lhs);
		ret *= rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator/=(Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] /= rhs[i];
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T>& operator/=(Array<N, T>& lhs, const T& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			lhs[i] /= rhs;
		}
		return lhs;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator/(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		Array<N, T> ret(lhs);
		ret /= rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> operator/(const Array<N, T>& lhs, const T& rhs)
	{
		Array<N, T> ret(lhs);
		ret /= rhs;
		return ret;
	}

	template <size_t N, typename T>
	inline bool operator==(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		for (size_t i = 0; i < N; i++) {
			if (lhs[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	template <size_t N, typename T>
	inline bool operator!=(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		return !(lhs == rhs);
	}

	////////////////////////////////////////////////////////////////////////////
	// method
	////////////////////////////////////////////////////////////////////////////
	template <size_t N, typename T>
	inline Array<N, T> ArrayMax(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		Array<N, T> ret;
		for (size_t i = 0; i < N; i++) {
			ret[i] = std::max(lhs[i], rhs[i]);
		}
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> ArrayMin(const Array<N, T>& lhs, const Array<N, T>& rhs)
	{
		Array<N, T> ret;
		for (size_t i = 0; i < N; i++) {
			ret[i] = std::min(lhs[i], rhs[i]);
		}
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> Lerp(const Array<N, T>& lhs, const Array<N, T>& rhs, const T& f)
	{
		return lhs * (1.0f - f) + rhs * f;
	}

	template <size_t N, typename T>
	inline Array<N, T> Clamp(const Array<N, T>& lhs, const T& lower, const T& upper)
	{
		Array<N, T> ret(lhs);
		for (size_t i = 0; i < N; i++) 
		{
			if (ret[i] < lower) {
				ret[i] = lower;
			}
			else if (ret[i] > upper) {
				ret[i] = upper;
			}
		}
		return ret;
	}

	template <size_t N, typename T>
	inline Array<N, T> Saturated(const Array<N, T>& lhs)
	{
		Array<N, T> ret(lhs);
		for (size_t i = 0; i < N; i++) 
		{
			if (ret[i] < 0) {
				ret[i] = 0;
			}
			else if (ret[i] > 1) {
				ret[i] = 1;
			}
		}
		return ret;
	}

	template <size_t N, typename T>
	inline bool NonZero(const Array<N, T>& v)
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (v[i]) {
				return true;
			}
		}
		return false;
	}
}

namespace std
{
	template<typename T, size_t N, size_t A>
	struct tuple_size<Cjing3D::Array<N, T, A> > : public integral_constant<size_t, N> {};

	template<size_t I, typename T, size_t N, size_t A>
	struct tuple_element<I, Cjing3D::Array<N, T, A> > { using type = T; };
}

namespace Cjing3D
{
	using U32x2 = Array<2, U32>;
	using U32x3 = Array<3, U32>;
	using U32x4 = Array<4, U32>;

	using F32x2 = Array<2, F32>;
	using F32x3 = Array<3, F32>;
	using F32x4 = Array<4, F32>;

	using I32x2 = Array<2, I32>;
	using I32x3 = Array<3, I32>;
	using I32x4 = Array<4, I32>;
}