#pragma once

#include "resource\converter.h"
#include "core\serialization\serializedObject.h"

namespace Cjing3D
{
	class ModelMetaObject : public SerializedObject
	{
	public:
		

	public:
		virtual void Serialize(JsonArchive& archive)const;
		virtual void Unserialize(JsonArchive& archive);
	};
}