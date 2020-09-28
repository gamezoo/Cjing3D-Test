#include "geometry.h"
#include "maths.h"

namespace Cjing3D {
namespace {
	
	inline constexpr bool Collision2D(const F32x2& box1Pos, const F32x2& box1Size, const F32x2& box2Pos, const F32x2& box2Size)
	{
		if (box1Pos[0] + box1Size[0] < box2Pos[0]) {
			return false;
		}
		else if (box1Pos[0] > box2Pos[0] + box2Size[0]) {
			return false;
		}
		else if (box1Pos[1] + box1Size[1] < box2Pos[1]) {
			return false;
		}
		else if (box1Pos[1] > box2Pos[1] + box2Size[1]) {
			return false;
		}
		else {
			return true;
		}
	}
}
	bool Rect::Intersects(const Rect& rect) const
	{
		return Collision2D(GetPos(), GetSize(), rect.GetPos(), rect.GetSize());
	}

	bool Rect::Intersects(const F32x2& point) const
	{
		return Collision2D(GetPos(), GetSize(), point, { 1.0f, 1.0f });
	}
}