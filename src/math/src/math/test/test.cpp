
#include "math\maths.h"

#define CATCH_CONFIG_MAIN
#include "catch\catch.hpp"

using namespace Cjing3D;

namespace
{
    static const F32 e = 0.0001f;

    template <size_t N, class T>
    bool RequireFunc(const Array<N, T>& l, const Array<N, T> r)
    {
        for (int i = 0; i < N; ++i)
        {
            if (!(fabs(l[i] - r[i]) < e)) {
                return false;
            }
        }
        return true;
    }

    bool RequireFunc(bool l, bool r)
    {
        return l == r;
    }

    bool RequireFunc(F32 l, F32 r)
    {
        return l == r;
    }
}

TEST_CASE("vec declare", "[vec]")
{
	U32 v = 10;
	U32x2 v2(v, 2u);
    U32x3 v3(v2, 1u);
    U32x4 v4(v2, 4u, 5u);
    U32x4 v5(v3, 6u);

    REQUIRE(RequireFunc(v2, U32x2(10u, 2u)));
    REQUIRE(RequireFunc(v3, U32x3(10u, 2u, 1u)));
    REQUIRE(RequireFunc(v4, U32x4(10u, 2u, 4u, 5u)));
    REQUIRE(RequireFunc(v5, U32x4(10u, 2u, 1u, 6u)));
}

TEST_CASE("vec +", "[vec]")
{
    U32x2 v1(1u, 2u);
    U32x2 v2(3u, 2u);
    U32x2 v3(4u, 2u);
    v3 += v2;

    U32x2 v4(2u, 10u);
    v4 += 10u;

    REQUIRE(RequireFunc(v1 + v2, U32x2(4u, 4u)));
    REQUIRE(RequireFunc(v3, U32x2(7u, 4u)));
    REQUIRE(RequireFunc(v4, U32x2(12u, 20u)));
}

TEST_CASE("vec -", "[vec]")
{
    I32x2 v1(1, 2);
    I32x2 v2(3, 2);
    I32x2 v3(4, 12);
    v3 -= v2;

    I32x2 v4(2, 15);
    v4 -= 10;

    REQUIRE(RequireFunc(v1 - v2, I32x2(-2, 0)));
    REQUIRE(RequireFunc(v3, I32x2(1, 10)));
    REQUIRE(RequireFunc(v4, I32x2(-8, 5)));
}

TEST_CASE("vec *", "[vec]")
{
    I32x2 v1(1, 2);
    I32x2 v2(3, 2);
    I32x2 v3(4, 12);
    v3 *= v2;

    I32x2 v4(2, 15);
    v4 *= 10;

    REQUIRE(RequireFunc(v1 * v2, I32x2(3, 4)));
    REQUIRE(RequireFunc(v3, I32x2(12, 24)));
    REQUIRE(RequireFunc(v4, I32x2(20, 150)));
}

TEST_CASE("vec /", "[vec]")
{
    I32x2 v1(12, 2);
    I32x2 v2(3, 2);
    I32x2 v3(21, 12);
    v3 /= v2;

    I32x2 v4(120, 150);
    v4 /= 10;

    REQUIRE(RequireFunc(v1 / v2, I32x2(4, 1)));
    REQUIRE(RequireFunc(v3, I32x2(7, 6)));
    REQUIRE(RequireFunc(v4, I32x2(12, 15)));
}


TEST_CASE("vec !=", "[vec]")
{
    I32x2 v1(12, 2);
    I32x2 v2(3, 2);
    I32x2 v3(12, 2);

    REQUIRE(RequireFunc(v1 != v2, true));
    REQUIRE(RequireFunc(v1 == v3, true));
    REQUIRE(RequireFunc(v1 == v2, false));
}

TEST_CASE("vec method", "[vec]")
{
    F32x2 v1(11.0f, 4.0f);
    F32x2 v2(3.0f,  2.0f);
    F32x2 v3(12.0f, 2.0f);
    F32x2 v4(-12.0f, -2.0f);

    REQUIRE(RequireFunc(Lerp(v1, v2, 0.5f), F32x2(7.0f, 3.0f)));
    REQUIRE(RequireFunc(Clamp(v1, 4.0f, 10.0f), F32x2(10.0f, 4.0f)));
    REQUIRE(RequireFunc(ArrayMin(v1, v2), F32x2(3.0f, 2.0f)));
    REQUIRE(RequireFunc(Saturated(v1), F32x2(1.0f, 1.0f)));
    REQUIRE(RequireFunc(Saturated(v4), F32x2(0.0f, 0.0f)));
}

TEST_CASE("mat declare", "[mat]")
{
    F32x4x4 mat1(1.0f);

    REQUIRE(RequireFunc(mat1.at(2), 1.0f));
}
