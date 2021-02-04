#include "reflection.h"
#include "core\container\staticArray.h"
#include "core\container\hashMap.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{
namespace ECS
{
	ComponentType ComponentType::INVALID;
}

namespace ECS::SceneReflection
{
	struct RegisterdComponentType
	{
		U32 mNameHash = 0;
	};

	struct SceneManager
	{
		I32 mComponentTypeCount = 0;
		StaticArray<RegisterdComponentType, ComponentType::MAX_TYPE_COUNT> mComponentTypes;
	};

	static SceneManager& GetManager()
	{
		static SceneManager manager;
		return manager;
	}

	ComponentType Cjing3D::ECS::SceneReflection::GetComponentType(const char* name)
	{
		auto& manager = GetManager();
		U32 hash = StringUtils::StringToHash(name);
		for (int i = 0; i < manager.mComponentTypes.size(); i++)
		{
			if (manager.mComponentTypes[i].mNameHash == hash) {
				return { (U32)i };
			}
		}
		return ComponentType::INVALID;
	}

	ComponentType RegisterComponentType(const char* name)
	{
		auto& manager = GetManager();
		U32 hash = StringUtils::StringToHash(name);
		for (int i = 0; i < manager.mComponentTypes.size(); i++)
		{
			if (manager.mComponentTypes[i].mNameHash == hash) {
				return { (U32)i };
			}
		}

		RegisterdComponentType& componentType = manager.mComponentTypes[manager.mComponentTypeCount++];
		componentType.mNameHash = hash;
		return { (U32)(manager.mComponentTypeCount - 1) };
	}
}
}