#pragma once

#include "core\common\definitions.h"
#include "core\container\dynamicArray.h"
#include "core\container\hashMap.h"
#include "core\jobsystem\jobsystem.h"
#include "core\helper\stringID.h"
#include "core\helper\reflection.h"

namespace Cjing3D
{
	class Universe;
	class ArchiveBase;

namespace ECS
{	
	using Entity = U32;
	static const Entity INVALID_ENTITY = 0;

	struct ComponentType
	{
	public:
		ComponentType(U32 index = MAX_TYPE_COUNT) : mTypeIndex(index) {}

		U32 mTypeIndex = 0;
		
		static const U32 MAX_TYPE_COUNT = 64;
		static ComponentType INVALID;

		bool operator==(const ComponentType& rhs) const {
			return mTypeIndex == rhs.mTypeIndex;
		}
		bool operator!=(const ComponentType& rhs) const {
			return mTypeIndex != rhs.mTypeIndex;
		}
		bool operator< (const ComponentType& rhs) const {
			return mTypeIndex < rhs.mTypeIndex;
		}
	};

	struct IComponent
	{
		Entity mEntity = INVALID_ENTITY;
		ComponentType mType;

		Entity GetEntity()const { return mEntity; }
		void SetEntity(Entity entity) { mEntity = entity; }

		virtual void Serialize(ArchiveBase& archive) {}
		virtual void Unserialize(ArchiveBase& archive)const {};
	};

	// Component 管理器，提供entity对Component映射
	class BaseComponentManager
	{
	public:
		BaseComponentManager() = default;
		virtual~BaseComponentManager() = default;
	};

	template<typename ComponentT>
	class ComponentManager : public BaseComponentManager
	{
	public:
		ComponentManager(size_t capacity = 0) :
			mCapacity(capacity)
		{
			mEntities.reserve(capacity);
			mComponents.reserve(capacity);
			mLookup.reserve(capacity);
		}

		ComponentManager(ComponentManager& rhs) = delete;
		ComponentManager& operator=(const ComponentManager& rhs) = delete;

		inline void Reserve(size_t capacity)
		{
			if (capacity > mCapacity)
			{
				mEntities.reserve(capacity);
				mComponents.reserve(capacity);
				mLookup.reserve(capacity);
			}
		}

		inline void Clear()
		{
			mEntities.clear();
			mComponents.clear();
			mLookup.clear();
		}

		inline ComponentT& Create(Entity entity)
		{
			Debug::CheckAssertion(entity != INVALID_ENTITY, "Invalid entity.");
			Debug::CheckAssertion(mLookup.find(entity) == nullptr, "The entity is already have component");
			Debug::CheckAssertion(mEntities.size() == mComponents.size());
			Debug::CheckAssertion(mLookup.size() == mComponents.size());

			// 必须要保证component和entity的index一致
			mLookup[entity] = mComponents.size();
			mEntities.push(entity);
			mComponents.emplace();
		
			return mComponents.back();
		}

		inline void Remove(Entity entity)
		{
			auto it = mLookup.find(entity);
			if (it != nullptr)
			{
				const auto index = *it;
				const Entity entity = mEntities[index];

				// 如果删除的不是最后的元素，则与最后的元素交换
				if (index < mComponents.size() - 1)
				{
					mComponents[index] = std::move(mComponents.back());
					mEntities[index] = mEntities.back();
					mLookup[mEntities.back()] = index;
				}

				mComponents.pop();
				mEntities.pop();
				mLookup.erase(entity);
			}
		}

		inline void RemoveAndKeepSorted(Entity entity)
		{
			auto it = mLookup.find(entity);
			if (it != nullptr)
			{
				const auto index = *it;
				const Entity entity = mEntities[index];

				// 如果删除的不是最后的元素，则将后面的元素往前移
				if (index < mComponents.size() - 1)
				{
					for (int i = index + 1; i < mComponents.size(); i++) {
						mComponents[i - 1] = std::move(mComponents[i]);
					}

					for (int i = index + 1; i < mEntities.size(); i++) {
						mEntities[i - 1] = mEntities[i];
						mLookup[mEntities[i - 1]] = i - 1;
					}
				}

				mComponents.pop();
				mEntities.pop();
				mLookup.erase(entity);
			}
		}

		inline void MoveInto(U32 from, U32 into)
		{
			if (from >= GetCount() || into >= GetCount() || from == into) {
				return;
			}

			auto targetComponent = std::move(mComponents[from]);
			auto targetEntity = mEntities[from];

			int dir = from < into ? 1 : -1;
			for (int i = from; i != into; i += dir) {
				int nextIndex = i + dir;
				mComponents[i] = std::move(mComponents[nextIndex]);
				mEntities[i] = mEntities[nextIndex];
				mLookup[mEntities[i]] = i;
			}

			mComponents[into] = std::move(targetComponent);
			mEntities[into] = targetEntity;
			mLookup[targetEntity] = into;
		}

		inline void MoveLastInto(U32 index)
		{
			// 将末尾的对象插入到指定位置
			MoveInto(GetCount() - 1, index);
		}

		inline void Merge(ComponentManager<ComponentT>& other)
		{
			const auto newSize = GetCount() + other.GetCount();
			mComponents.reserve(newSize);
			mEntities.reserve(newSize);
			mLookup.reserve(newSize);

			for (size_t i = 0; i < other.GetCount(); i++)
			{
				Entity entity = other.GetEntityByIndex(i);
				if (Contains(entity) == false)
				{
					mEntities.push(entity);
					mLookup[entity] = mComponents.size();
					mComponents.push(std::move(other.mComponents[i]));
				}
			}

			// merge的时候，other的component不需要释放
			other.Clear(false);
		}

		inline bool Contains(Entity entity)
		{
			return mLookup.find(entity) != nullptr;
		}

		inline void DuplicateComponent(Entity oldEntity, Entity newEntity)
		{
			if (!Contains(oldEntity) || newEntity == INVALID_ENTITY) {
				return;
			}

			auto oldComponent = GetComponent(oldEntity);
			Create(newEntity, oldComponent);
		}

		inline ComponentT* GetOrCreate(Entity entity)
		{
			auto ret = GetComponent(entity);
			if (ret != nullptr) {
				return ret;
			}
			return &Create(entity);
		}

		inline ComponentT* GetComponent(Entity entity)
		{
			auto it = mLookup.find(entity);
			if (it != nullptr)
			{
				return &mComponents[*it];
			}
			return nullptr;
		}

		inline const ComponentT* GetComponent(Entity entity)const
		{
			auto it = mLookup.find(entity);
			if (it != nullptr)
			{
				return &mComponents[*it];
			}
			return nullptr;
		}

		inline ComponentT* GetComponentByIndex(U32 index)
		{
			Debug::CheckAssertion(index >= 0 && index < mComponents.size());
			return &mComponents[index];
		}

		inline const ComponentT* GetComponentByIndex(U32 index)const
		{
			Debug::CheckAssertion(index >= 0 && index < mComponents.size());
			return &mComponents[index];
		}

		inline std::vector<ComponentT>& GetComponents()
		{
			return mComponents;
		}

		inline size_t GetEntityIndex(Entity entity)
		{
			auto it = mLookup.find(entity);
			if (it != nullptr)
			{
				return *it;
			}
			return ~0;
		}

		inline size_t GetCount() const
		{
			return mComponents.size();
		}

		inline bool Empty()const
		{
			return mComponents.empty();
		}

		inline Entity GetEntityByIndex(size_t index)const
		{
			if (index >= 0 && index < mEntities.size())
			{
				return mEntities[index];
			}
			return INVALID_ENTITY;
		}

		inline std::vector<Entity>& GetEntities()
		{
			return mEntities;
		}

		inline ComponentT operator[](size_t index)
		{
			return mComponents[index];
		}

		inline const ComponentT operator[](size_t index) const
		{
			return mComponents[index];
		}

	private:
		DynamicArray<Entity> mEntities;
		DynamicArray<ComponentT> mComponents;
		HashMap<Entity, size_t> mLookup;
		I32 mCapacity;
	};

	class IScene
	{
	public:
		virtual~IScene() {}

		virtual void Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void Update(F32 dt) = 0;
		virtual void LateUpdate(F32 dt) = 0;
		virtual void Clear() = 0;
		virtual Universe& GetUniverse() = 0;
	};

	using SystemType = I32;

	namespace Impl 
	{
		template <typename... Args>
		struct SystemTypeListAdder;

		template<typename SystemT>
		struct SystemTypeListAdder<SystemT>
		{
			static void Call(DynamicArray<SystemType>& list)
			{
				list.push(Reflection::TypeID<SystemT>());
			}
		};

		template<typename SystemT, typename... OtherSystems>
		struct SystemTypeListAdder<SystemT, OtherSystems...>
		{
			static void Call(DynamicArray<SystemType>& list)
			{
				list.push(Reflection::TypeID<SystemT>());
				SystemTypeListAdder<OtherSystems...>::Call(list);
			}
		};
	} 

	class ISystem
	{
	public:
		ISystem() {}
		virtual ~ISystem() {}

		virtual void Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs) = 0;
		virtual SystemType GetTypeID()const = 0;

		const DynamicArray<SystemType>& GetDependencies()const { 
			return mDependencies; 
		}
		const DynamicArray<SystemType>& GetDependents()const {
			return mDependents;
		}

	protected:
		template<typename... Args>
		void DeclareDependencies() {
			Impl::SystemTypeListAdder<Args...>::Call(mDependencies);
		}

		template<typename... Args>
		void DeclareDependents() {
			Impl::SystemTypeListAdder<Args...>::Call(mDependents);
		}

	private:
		DynamicArray<SystemType> mDependencies;
		DynamicArray<SystemType> mDependents;
	};

	template<typename SystemT>
	class System : public ISystem
	{
	public:
		System() : ISystem() {}
		virtual ~System() {}

		SystemType GetTypeID()const override
		{
			return Reflection::TypeID<SystemT>();
		}
	};
}

#define DECLARE_SYSTEM(type) \
	class type : public ECS::System<type> { \
	public:	\
		##type(); \
		void Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)override; \
	};

}