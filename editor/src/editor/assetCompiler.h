#pragma once

#include "resource\resourceManager.h"
#include "core\container\hashMap.h"

namespace Cjing3D
{
	class GameEditor;

	class AssetCompiler
	{
	public:
		AssetCompiler(GameEditor& gameEditor);
		~AssetCompiler();

		void SetupAssets();
		void Update(F32 deltaTime);
		void CompileResources();

		Signal<void()>& GetOnListChanged();
		
		struct ResourceItem
		{
			Path mResPath;
			U32 mResDirHash;
			ResourceType mResType;
		};
		const HashMap<U32, ResourceItem>& MapResources()const;
		void UnmapResources();

	private:
		class AssetCompilerImpl* mImpl = nullptr;
	};
}