#pragma once

#include "core\serialization\serializedObject.h"

namespace Cjing3D
{
	class TestMetaObject : public SerializedObject
	{
	public:
		int mTestValue = 0;

	public:
		virtual void Serialize(JsonArchive& archive);
		virtual void Unserialize(JsonArchive& archive)const;
	};
}