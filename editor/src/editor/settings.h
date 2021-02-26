#pragma once

#include "core\common\common.h"
#include "core\string\string.h"

namespace Cjing3D
{
	class JsonArchive;

	struct EditorSettings
	{
		String mImGuiIni;

		bool Load(JsonArchive& archive);
		void Save(JsonArchive& archive);
	};
}