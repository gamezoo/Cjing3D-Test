#pragma once

#include "resource\resourceManager.h"

namespace Cjing3D
{
	class GameEditor;

	class AssertCompiler
	{
	public:
		AssertCompiler(GameEditor& gameEditor);
		~AssertCompiler();

		void SetupAsserts();
		void Update(F32 deltaTime);

	private:
		class AssertCompilerImpl* mImpl = nullptr;
	};
}