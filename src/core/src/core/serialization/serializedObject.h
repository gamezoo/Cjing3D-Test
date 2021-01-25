#pragma once

#include "archive.h"

namespace Cjing3D
{
	class JsonArchive;

	class SerializedObject
	{
	public:
		// json serialize
		virtual void Serialize(JsonArchive& archive)const {};
		virtual void Unserialize(JsonArchive& archive) {};
	};
}