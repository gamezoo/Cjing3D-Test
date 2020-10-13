#pragma once

#include <assert.h>

namespace Cjing3D
{
	template<typename T>
	class Span
	{
	public:
		Span() = default;
		Span(T* begin, size_t len) :
			mBegin(begin),
			mEnd(begin + len)
		{}
		Span(T* begin, T* end) :
			mBegin(begin),
			mEnd(end)
		{}
		template<size_t N>
		Span(T(&data)[N]) :
			mBegin(data),
			mEnd(data + N)
		{}

		operator Span<const T>() const
		{
			return Span<const T>(mBegin, mEnd);
		}

		T& operator[](const size_t idx) const
		{
			assert(mBegin + idx < mEnd);
			return mBegin[idx];
		}
		bool empty()const {
			return length() <= 0;
		}

		size_t length()const { 
			return size_t(mEnd - mBegin);
		}
		T* begin()const {
			return mBegin;
		}
		T* end()const {
			return mEnd;
		}
		T* data() {
			return mBegin;
		}

	private:
		T* mBegin = nullptr;
		T* mEnd   = nullptr;
	};
}