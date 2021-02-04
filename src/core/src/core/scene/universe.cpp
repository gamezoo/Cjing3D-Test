#include "universe.h"

namespace Cjing3D
{
	using namespace ECS;

	/// ////////////////////////////////////////////////////////////////////////////////
	// systems

	TransformSystem::TransformSystem() {}

	void TransformSystem::Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)
	{ 
		auto transforms = universe.GetComponents<Transform>(SceneReflection::GetComponentType("Transform"));
		for (int i = 0; i < transforms->GetCount(); i++)
		{
			Transform* transform = transforms->GetComponentByIndex(i);
			if (transform != nullptr ) {
				transform->Update();
			}
		}
		waitJobs = true;
	}

	HierarchySystem::HierarchySystem()
	{
		DeclareDependencies<TransformSystem>();
	}

	void HierarchySystem::Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)
	{
		auto transforms = universe.GetComponents<Transform>(SceneReflection::GetComponentType("Transform"));
		auto hierarchyMgr = universe.GetComponents<EntityHierarchy>(SceneReflection::GetComponentType("Hierarchy"));

		// 次序更新每个Hierarchy，将Transform根据层级关系更新WorldMatrix
		for (int i = 0; i < hierarchyMgr->GetCount(); i++)
		{
			const EntityHierarchy* hierarchy = hierarchyMgr->GetComponentByIndex(i);

			Transform* childTrans = transforms->GetOrCreate(hierarchyMgr->GetEntityByIndex(i));
			Transform* parentTrans = transforms->GetOrCreate(hierarchy->mParent);
			if (childTrans != nullptr && parentTrans != nullptr) {
				childTrans->UpdateFromParent(*parentTrans);
			}
		}
	}

	/// ////////////////////////////////////////////////////////////////////////////////
	// Universe

	Universe::Universe(I32 capacity)
	{
		// init default systems
		mTransforms = RegisterComponents<Transform>(SceneReflection::RegisterComponentType("Transform"));
		mHierarchy = RegisterComponents<EntityHierarchy>(SceneReflection::RegisterComponentType("Hierarchy"));
		mNames = RegisterComponents<EntityName>(SceneReflection::RegisterComponentType("Name"));

		RegisterSystem(CJING_NEW(TransformSystem));
		RegisterSystem(CJING_NEW(HierarchySystem));

		ResizeUniverse(capacity);
	}

	Universe::~Universe()
	{
		for (auto system : mSystems)
		{
			if (system.mSystem != nullptr) {
				CJING_SAFE_DELETE(system.mSystem);
			}
		}
		for (auto kvp : mComponentMangers) {
			CJING_SAFE_DELETE(kvp.second);
		}
		mComponentMangers.clear();
	}

	void Universe::Update(F32 deltaTime)
	{
		// update system
		JobSystem::JobHandle jobHandle = JobSystem::INVALID_HANDLE;
		bool waitJobs = false;
		for (auto system : mOrderedSystems)
		{
			// update system
			system->mSystem->Update(*this, jobHandle, waitJobs);

			if (waitJobs)
			{
				JobSystem::Wait(&jobHandle);
				jobHandle = JobSystem::INVALID_HANDLE;
				waitJobs = false;
			}
		}

		if (jobHandle != JobSystem::INVALID_HANDLE) {
			JobSystem::Wait(&jobHandle);
		}
	}

	DynamicArray<UniquePtr<ECS::IScene>>& Universe::GetScenes()
	{
		return mScenes;
	}

	void Universe::AddScene(UniquePtr<ECS::IScene>&& scene)
	{
		mScenes.push(std::move(scene));
	}

	ECS::Entity Universe::CreateEntity(const char* name)
	{
		ECS::Entity entity = ECS::INVALID_ENTITY;

		// all entities is used, resize universe capacity
		if (mFreeListHead == nullptr) {
			ResizeUniverse(mCapacity * 2);
		}
		I32 freeIndex = mFreeListHead->mIndex;
		mFreeListHead = mFreeListHead->mNext;

		// create new entity
		entity = freeIndex;
		auto& entityInst = mEntities[freeIndex];
		entityInst.mIsValid = true;

		if (name != nullptr && name[0] != '\0')  {
			mNames->Create(entity).mName = name;
		}

		return entity;
	}

	void Universe::DestroyEntity(ECS::Entity entity)
	{
		if (entity == INVALID_ENTITY) {
			return;
		}

		EntityInst& entityInst = mEntities[entity];

		// clear hierarchy
		EntityDetachChildren(entity);
		EntityDetach(entity);

		// clear components

		// clear name
		if (mNames->Contains(entity)) {
			mNames->Remove(entity);
		}

		entityInst.mIsValid = false;

		// reset free list head
		auto nextNode = mFreeListHead;
		mFreeListHead = &mFreeList[entity];
		mFreeListHead->mNext = nextNode;

		if (nextNode != nullptr) {
			nextNode->mPrev = mFreeListHead;
		}

		// invoke callbacks
		OnEntityDestroyed(entity);
	}

	Entity Universe::GetFirstChild(Entity entity)const
	{
		auto hierarchy = mHierarchy->GetComponent(entity);
		return hierarchy != nullptr ? hierarchy->mFirstChild : INVALID_ENTITY;
	}

	DynamicArray<Entity> Universe::GetChildren(Entity entity)
	{
		DynamicArray<Entity> children;
		EntityHierarchy* hierarchy = mHierarchy->GetComponent(entity);
		if (hierarchy == nullptr) {
			return children;
		}

		Entity childEntity = hierarchy->mFirstChild;
		while (childEntity != INVALID_ENTITY)
		{
			children.push(childEntity);

			auto siblingHierarchy = mHierarchy->GetComponent(childEntity);
			childEntity = siblingHierarchy != nullptr ? siblingHierarchy->mNextSibling : INVALID_ENTITY;
		}
		return children;
	}

	Entity Universe::GetNextSlibling(Entity entity) const
	{
		return Entity();
	}

	void Universe::EntityAttach(Entity child, Entity parent, bool isChildAlreadyInLocalSpace)
	{
		if ((child == parent) || (child == INVALID_ENTITY && parent == INVALID_ENTITY)) {
			return;
		}
		
		if (mHierarchy->Contains(child)) {
			EntityDetach(child);
		}

		if (parent == INVALID_ENTITY) {
			return;
		}

		auto& hierarchy = mHierarchy->Create(child);
		hierarchy.mParent = parent;

		auto& parentHierarchy = *mHierarchy->GetOrCreate(parent);
		hierarchy.mNextSibling = parentHierarchy.mFirstChild;
		parentHierarchy.mFirstChild = child;

		// check ordering of tree which keep parent before child
		if (mHierarchy->GetCount() > 1)
		{
			for (int i = mHierarchy->GetCount() - 1; i > 0; i--)
			{
				auto parentHierachy = mHierarchy->GetEntityByIndex(i);
				for (int j = 0; j < i; j++)
				{
					auto childHierachy = mHierarchy->GetComponentByIndex(j);
					if (childHierachy->mParent == parentHierachy)
					{
						mHierarchy->MoveInto(i, j);
						i++;	// check same index again
						break;
					}
				}
			}
		}
		// update transfroms
		Transform* childTrans = mTransforms->GetOrCreate(child);
		Transform* parentTrans = mTransforms->GetOrCreate(parent);

		// convert to local space
		if (!isChildAlreadyInLocalSpace)
		{
			auto world = childTrans->mWorld;
			childTrans->UpdateByMatrix(parentTrans->GetInvertedWorldMatrix());
			childTrans->Update();
			childTrans->mWorld = world;
		}
		else
		{
			childTrans->UpdateFromParent(*parentTrans);
		}
	}

	void Universe::EntityDetach(Entity entity)
	{
		EntityHierarchy* hierarchy = mHierarchy->GetComponent(entity);
		if (hierarchy == nullptr) {
			return;
		}
		if (hierarchy->mParent != INVALID_ENTITY)
		{
			auto parentHierarchy = mHierarchy->GetComponent(hierarchy->mParent);
			if (parentHierarchy != nullptr)
			{
				// 将Transfrom恢复为world space
				Transform* childTrans = mTransforms->GetComponent(entity);
				if (childTrans) {
					childTrans->WorldMatrixToLocal();
				}

				// 从parentHierarchy的childList中移除child
				Entity* childEntity = &parentHierarchy->mFirstChild;
				while (childEntity != nullptr && *childEntity != INVALID_ENTITY)
				{
					if (*childEntity == entity)
					{
						*childEntity = GetNextSlibling(entity);
						break;
					}
				
					auto siblingHierarchy = mHierarchy->GetComponent(*childEntity);
					childEntity = siblingHierarchy != nullptr ? &siblingHierarchy->mNextSibling : nullptr;
				}

				// check parent hierarchy
				if (parentHierarchy->mFirstChild == INVALID_ENTITY && parentHierarchy->mParent == INVALID_ENTITY) {
					mHierarchy->RemoveAndKeepSorted(entity);
				}
			}
		}

		if (hierarchy->mFirstChild == INVALID_ENTITY && hierarchy->mParent == INVALID_ENTITY) {
			mHierarchy->RemoveAndKeepSorted(entity);
		}
	}

	void Universe::EntityDetachChildren(Entity entity)
	{
		for (auto child = GetFirstChild(entity); child != INVALID_ENTITY; child = GetFirstChild(entity)) {
			EntityDetach(child);
		}
	}

	ECS::Entity Universe::FindEntityByName(Entity parent, const char* name)const
	{
		if (parent == INVALID_ENTITY)
		{
			for (int i = 0; i < mNames->GetCount(); i++)
			{
				if (EqualString((*mNames)[i].mName, name)) {
					return mNames->GetEntityByIndex(i);
				}
			}
		}
		else
		{
			const EntityHierarchy* hierarchy = mHierarchy->GetComponent(parent);
			if (hierarchy == nullptr) {
				INVALID_ENTITY;
			}

			Entity childEntity = hierarchy->mFirstChild;
			while (childEntity != INVALID_ENTITY)
			{
				auto entityName = mNames->GetComponent(childEntity);
				if (entityName != nullptr && EqualString(entityName->mName, name)) {
					return childEntity;
				}

				auto siblingHierarchy = mHierarchy->GetComponent(childEntity);
				childEntity = siblingHierarchy != nullptr ? siblingHierarchy->mNextSibling : INVALID_ENTITY;
			}
		}
		return INVALID_ENTITY;
	}

	bool Universe::HasEntity(Entity entity) const
	{
		return entity >= 0 && entity < mEntities.size() && mEntities[entity].mIsValid;
	}

	void Universe::SetEntityTransform(Entity entity, const Transform& transform)
	{
		auto entityTrans = mTransforms->GetOrCreate(entity);
		*entityTrans = transform;
		entityTrans->SetDirty(true);
	}

	void Universe::SetEntityName(Entity entity, const char* name)
	{
		auto entityName = mNames->GetOrCreate(entity);
		CopyString(entityName->mName.toSpan(), name);
	}

	const char* Universe::GetEntityName(Entity entity)const
	{
		auto entityName = mNames->GetComponent(entity);
		return entityName != nullptr ? entityName->mName.data() : nullptr;
	}

	void Universe::RegisterSystem(ECS::ISystem* system)
	{
		SystemInst& systemInst = mSystems.emplace();
		systemInst.mSystem = system;
		
		// 重新设置各个system的依赖
		auto& dependencies = system->GetDependencies();
		auto& dependents = system->GetDependents();
		for (auto& other : mSystems)
		{
			if (&other == &systemInst) {
				continue;
			}

			// 计算当前system对其他的依赖
			if (std::find(dependencies.begin(), dependencies.end(), other.mSystem->GetTypeID()) != dependencies.end()) {
				systemInst.mDependencies.push(&other);
			}

			// 计算是否有其他system依赖当前项
			if (std::find(dependents.begin(), dependents.end(), other.mSystem->GetTypeID()) != dependents.end()) {
				other.mDependents.push(&systemInst);
			}
		}

		CalculateSystemShedule();
	}

	void Universe::CalculateSystemShedule()
	{
		mOrderedSystems.clear();

		for (auto& system : mSystems) {
			system.mIsVisited = false;
		}

		for (auto& system : mSystems) {
			SortSystem(system);
		}
	}

	void Universe::SortSystem(SystemInst& system)
	{
		if (system.mIsVisited) {
			return;
		}
		system.mIsVisited = true;

		for (auto dep : system.mDependencies) {
			SortSystem(*dep);
		}

		mOrderedSystems.push(&system);
	}

	void Universe::ResizeUniverse(I32 newSize)
	{
		mEntities.resize(newSize);
		mFreeList.resize(newSize);

		mCapacity = newSize;
		ResizeFreeList();
	}

	void Universe::ResizeFreeList()
	{
		mFreeListHead = nullptr;
		for (int i = mCapacity - 1; i >= 0; i--)
		{
			mFreeList[i].mIndex = i;
			if (!mEntities[i].mIsValid)
			{
				auto node = &mFreeList[i];
				node->mNext = mFreeListHead;

				if (mFreeListHead != nullptr) {
					mFreeListHead->mPrev = node;
				}
				mFreeListHead = node;
			}
		}

		Debug::CheckAssertion(mFreeListHead != nullptr, 
			"Failed to resize free list of universe.");
	}
}