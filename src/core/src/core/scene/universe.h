#pragma once

#include "ecs.h"
#include "reflection.h"
#include "core\memory\memory.h"
#include "core\container\dynamicArray.h"
#include "core\signal\connectionList.h"
#include "math\transform.h"

namespace Cjing3D
{
	const I32 DEFAULT_UNIVERSE_CAPACITY = 128;
	const I32 ENTITY_NAME_MAX_LENGTH = 64;

	struct EntityName
	{
		StaticString<ENTITY_NAME_MAX_LENGTH> mName;
	};

	struct EntityHierarchy
	{
		ECS::Entity mParent = ECS::INVALID_ENTITY;
		ECS::Entity mFirstChild = ECS::INVALID_ENTITY;
		ECS::Entity mNextSibling = ECS::INVALID_ENTITY;
	};

	class TransformSystem : public ECS::System<TransformSystem>
	{
	public:
		TransformSystem();
		void Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)override;
	};

	class HierarchySystem : public ECS::System<HierarchySystem>
	{
	public:
		HierarchySystem();
		void Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)override;
	};

	class Universe
	{
	public:
		Universe(I32 capacity = DEFAULT_UNIVERSE_CAPACITY);
		~Universe();

		void Update(F32 deltaTime);

		// scenes
		DynamicArray<UniquePtr<ECS::IScene>>& GetScenes();
		void AddScene(UniquePtr<ECS::IScene>&& scene);

		// entity 
		ECS::Entity CreateEntity(const char* name = nullptr);
		void DestroyEntity(ECS::Entity entity);
		ECS::Entity GetFirstChild(ECS::Entity entity)const;
		DynamicArray<ECS::Entity> GetChildren(ECS::Entity entity);
		ECS::Entity GetNextSlibling(ECS::Entity entity)const;

		void EntityAttach(ECS::Entity child, ECS::Entity parent, bool isChildAlreadyInLocalSpace = false);
		void EntityDetach(ECS::Entity entity);
		void EntityDetachChildren(ECS::Entity entity);
		ECS::Entity FindEntityByName(ECS::Entity parent, const char* name)const;
		bool HasEntity(ECS::Entity entity)const;

		void SetEntityTransform(ECS::Entity entity, const Transform& transform);
		void SetEntityName(ECS::Entity entity, const char* name);
		const char* GetEntityName(ECS::Entity entity)const;

		void RegisterSystem(ECS::ISystem* system);

		Signal<void(ECS::Entity entity)> OnEntityDestroyed;

		template<typename ComponentT>
		ECS::ComponentManager<ComponentT>* RegisterComponents(const ECS::ComponentType& type)
		{
			auto it = mComponentMangers.find(type);
			if (it != mComponentMangers.end()) {
				return reinterpret_cast<ECS::ComponentManager<ComponentT>*>(it.value());
			}

			auto manager = CJING_NEW(ECS::ComponentManager<ComponentT>);
			mComponentMangers.insert(type, manager);
			return manager;
		}
		template<typename ComponentT>
		ECS::ComponentManager<ComponentT>* GetComponents(const ECS::ComponentType& type)
		{
			auto it = mComponentMangers.find(type);
			if (it != mComponentMangers.end()) {
				return reinterpret_cast<ECS::ComponentManager<ComponentT>*>(it.value());
			}
			return nullptr;
		}

	private:
		I32 mCapacity = 0;
		DynamicArray<UniquePtr<ECS::IScene>> mScenes;

		struct EntityInst
		{
			bool mIsValid = false;
		};
		DynamicArray<EntityInst> mEntities;

		void ResizeUniverse(I32 newSize);

		// freeList
		struct FreeListNode
		{
			I32 mIndex = -1;
			FreeListNode* mPrev = nullptr;
			FreeListNode* mNext = nullptr;
		};
		void ResizeFreeList();
		DynamicArray<FreeListNode> mFreeList;
		FreeListNode* mFreeListHead = nullptr;

		// callbacks
		ConnectionList mConnectEntityDestroyed;

		// systems
		struct SystemInst
		{
			ECS::ISystem* mSystem = nullptr;
			DynamicArray<SystemInst*> mDependencies;
			DynamicArray<SystemInst*> mDependents;
			bool mIsVisited = false;
		};
		DynamicArray<SystemInst> mSystems;
		DynamicArray<SystemInst*> mOrderedSystems;

		void CalculateSystemShedule();
		void SortSystem(SystemInst& system);

		// system components
		Map<ECS::ComponentType, ECS::BaseComponentManager*> mComponentMangers;

		ECS::ComponentManager<EntityName>* mNames = nullptr;
		ECS::ComponentManager<EntityHierarchy>* mHierarchy = nullptr;
		ECS::ComponentManager<Transform>* mTransforms = nullptr;
	};
}