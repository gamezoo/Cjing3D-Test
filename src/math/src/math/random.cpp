#include "random.h"

#include <random>

namespace Cjing3D
{
namespace Random
{
	std::random_device  rand_dev;
	std::mt19937		engine(rand_dev());

	U32 GetRandomInt(U32 x)
	{
		return GetRandomInt(0, x);
	}

	U32 GetRandomInt(U32 x, U32 y)
	{
		std::uniform_int_distribution<U32> dist{};
		using param_type = std::uniform_int_distribution<U32>::param_type;
		return dist(engine, param_type{ x,y });
	}

	F32 GetRandomFloat(F32 x)
	{
		return GetRandomFloat(0.0f, x);
	}

	F32 GetRandomFloat(F32 x, F32 y)
	{
		std::uniform_real_distribution<F32> dist{};
		using param_type = std::uniform_real_distribution<F32>::param_type;
		return dist(engine, param_type{ x,y });
	}

	F64 GetRandomDouble(F64 x, F64 y)
	{
		std::uniform_real_distribution<F64> dist{};
		using param_type = std::uniform_real_distribution<F64>::param_type;
		return dist(engine, param_type{ x,y });
	}

	F64 GetRandomDouble(F64 x)
	{
		return GetRandomDouble(0.0f, x);
	}
}
}