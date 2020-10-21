#include "idGenerator.h"

namespace Cjing3D {
	U32 IDGenerator::mCurrentGUID = 0;

	IDGenerator & IDGenerator::GetInstance()
	{
		static IDGenerator instance;
		return instance;
	}

	U32 IDGenerator::GenerateNextGUID() const
	{
		return mCurrentGUID++;
	}

	U32 IDGenerator::GenerateRandomGUID() const
	{
		return U32(Random::GetRandomInt(1, INT_MAX));
	}
}