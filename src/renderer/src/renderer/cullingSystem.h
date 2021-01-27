#pragma once

#include "renderer\definitions.h"

namespace Cjing3D
{
	enum CULLING_FLAG  
	{
		CULLING_FLAG_OBJECTS,
		CULLING_FLAG_ALL,
		CULLING_FLAG_COUNT
	};

	struct CullResult
	{

	};

	class CullingSystem
	{
	public:
		CullingSystem();
		~CullingSystem();
	};
}