#pragma once

#include "resource\converter.h"

namespace Cjing3D
{
	class ShaderEditor
	{
	public:
		void OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res);
	};
}