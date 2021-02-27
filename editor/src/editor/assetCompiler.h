#pragma once

#include "resource\resourceManager.h"
#include "core\container\hashMap.h"

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

		Signal<void()>& GetOnListChanged();
		
		struct ResourceItem
		{
			Path mResPath;
			Path mResDir;
			ResourceType mResType;
		};
		const HashMap<U32, ResourceItem>& MapResources()const;
		void UnmapResources();

	private:
		class AssertCompilerImpl* mImpl = nullptr;
	};
}