#pragma once

#include "archive.h"

namespace Cjing3D
{
	class JsonArchive;

	class SerializedObject
	{
	public:
		// json serialize
		virtual void Serialize(JsonArchive& archive) {};
		virtual void Unserialize(JsonArchive& archive)const {};
	};
}