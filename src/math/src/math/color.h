#pragma once

#include "vec.h"

namespace Cjing3D {

	class Color4
	{
	public:
		U32 mRGBA = 0;

		constexpr Color4(U32 color) : mRGBA(color) {};
		constexpr Color4(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0) : mRGBA(r << 0 | g << 8 | b << 16 | a << 24) {};
		constexpr Color4(const F32x4& value) { Convert(value); }

		constexpr uint8_t GetR()const { return (mRGBA >> 0) & 0xFF; }
		constexpr uint8_t GetG()const { return (mRGBA >> 8) & 0xFF; }
		constexpr uint8_t GetB()const { return (mRGBA >> 16) & 0xFF; }
		constexpr uint8_t GetA()const { return (mRGBA >> 24) & 0xFF; }

		constexpr void SetR(uint8_t v) { *this = Color4(v, GetG(), GetB(), GetA());}
		constexpr void SetG(uint8_t v) { *this = Color4(GetA(), v, GetB(), GetA()); }
		constexpr void SetB(uint8_t v) { *this = Color4(GetA(), GetG(), v, GetA()); }
		constexpr void SetA(uint8_t v) { *this = Color4(GetA(), GetG(), GetB(), v); }

		constexpr void SetFloatR(F32 v) { *this = Color4(uint8_t(v * 255), GetG(), GetB(), GetA()); }
		constexpr void SetFloatG(F32 v) { *this = Color4(GetA(), uint8_t(v * 255), GetB(), GetA()); }
		constexpr void SetFloatB(F32 v) { *this = Color4(GetA(), GetG(), uint8_t(v * 255), GetA()); }
		constexpr void SetFloatA(F32 v) { *this = Color4(GetA(), GetG(), GetB(), uint8_t(v * 255)); }

		U32 GetRGBA()const { return mRGBA; }

		// convert to 0.0-1.0
		constexpr F32x3 ToFloat3() const
		{
			return F32x3(
				((mRGBA >> 0) & 0xff) / 255.0f,
				((mRGBA >> 8) & 0xff) / 255.0f,
				((mRGBA >> 16) & 0xff) / 255.0f
			);
		}

		// convert to 0.0-1.0
		constexpr F32x4 ToFloat4() const
		{
			return F32x4(
				((mRGBA >> 0) & 0xff) / 255.0f,
				((mRGBA >> 8) & 0xff) / 255.0f,
				((mRGBA >> 16) & 0xff) / 255.0f,
				((mRGBA >> 24) & 0xff) / 255.0f
			);
		}

		static constexpr Color4 Convert(const F32x3& value)
		{
			return Color4(
				(uint8_t)(value[0] * 255.0f),
				(uint8_t)(value[1] * 255.0f),
				(uint8_t)(value[2] * 255.0f)
			);
		}

		static constexpr Color4 Convert(const F32x4& value)
		{
			return Color4(
				(uint8_t)(value[0] * 255.0f),
				(uint8_t)(value[1] * 255.0f),
				(uint8_t)(value[2] * 255.0f),
				(uint8_t)(value[3] * 255.0f)
			);
		}

		static constexpr Color4 Convert(const XMFLOAT4& value)
		{
			return Color4(
				(uint8_t)(value.x * 255.0f),
				(uint8_t)(value.y * 255.0f),
				(uint8_t)(value.z * 255.0f),
				(uint8_t)(value.w * 255.0f)
			);
		}

		static constexpr Color4 Red() { return Color4(255, 0, 0, 255); }
		static constexpr Color4 Green() { return Color4(0, 255, 0, 255); }
		static constexpr Color4 Blue() { return Color4(0, 0, 255, 255); }
		static constexpr Color4 Black() { return Color4(0, 0, 0, 255); }
		static constexpr Color4 White() { return Color4(255, 255, 255, 255); }
		static constexpr Color4 Yellow() { return Color4(255, 255, 0, 255); }
		static constexpr Color4 Pink() { return Color4(255, 0, 255, 255); }

		bool operator==(const Color4& color)const { return mRGBA == color.mRGBA; }
		bool operator!=(const Color4& color)const { return mRGBA != color.mRGBA; }
	};
}