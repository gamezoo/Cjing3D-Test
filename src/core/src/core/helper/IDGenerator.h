#pragma once

#include "math\maths.h"

namespace Cjing3D {

#define GENERATE_ID IDGenerator::GetInstance().GenerateNextGUID()
#define GENERATE_RANDOM_ID IDGenerator::GetInstance().GenerateRandomGUID();

class IDGenerator
{
public:
	static IDGenerator& GetInstance();
	U32 GenerateNextGUID()const;
	U32 GenerateRandomGUID()const;

private:
	IDGenerator() = default;
	static U32 mCurrentGUID;
};
}