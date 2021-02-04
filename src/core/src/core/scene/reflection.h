#pragma once

#include "ecs.h"
#include "core\container\map.h"
#include "core\helper\stringID.h"
#include "core\helper\reflection.h"

#include <type_traits>
#include <functional>

namespace Cjing3D
{
namespace ECS::SceneReflection
{ 
	///////////////////////////////////////////////////////////////////////////////////////
	// ComponentType
	///////////////////////////////////////////////////////////////////////////////////////
	ComponentType GetComponentType(const char* name);
	ComponentType RegisterComponentType(const char* name);

	///////////////////////////////////////////////////////////////////////////////////////
	// Scene reflection
	///////////////////////////////////////////////////////////////////////////////////////

	namespace Impl
	{
		template<typename SceneT>
		struct ComponentInfo
		{
			StringID mIdentifier;
			void(* const Create)(SceneT* scene, Entity entity);
			void(* const Destroy)(SceneT* scene, Entity entity);
		};

		template<typename SceneT>
		struct SceneInfo
		{
			StringID mIdentifier;
			Map<StringID, ComponentInfo<SceneT>*> mComponents;
		};

		template<typename T>
		struct InfoFactory
		{
			inline static SceneInfo<T>* mSceneInfo = nullptr;
			inline static SceneInfo<T>* Resolve()noexcept;
		};
	}

	template<typename T>
	inline Impl::SceneInfo<T>* Impl::InfoFactory<T>::Resolve()noexcept
	{
		if (mSceneInfo == nullptr)
		{
			static SceneInfo<T> sceneInfo{
				StringID::EMPTY
			};
			mSceneInfo = &sceneInfo;
		}
		return mSceneInfo;
	}

	template<typename SceneT>
	class SceneFactory;

	template<typename ComponentT, typename SceneT>
	class ComponentFactory
	{
	private:
		Reflection::MetaFactory<ComponentT> mMetaInfo;

	public:
		template<auto Creator, auto Destroyer>
		ComponentFactory BeginComponent(const StringID& uid)
		{
			Impl::SceneInfo<SceneT>* sceneInfo = Impl::InfoFactory<SceneT>::Resolve();
			static Impl::ComponentInfo<SceneT> componentInfo{
				uid,
				[](SceneT* scene, Entity entity) {
					(scene->*static_cast<void (SceneT::*)(Entity)>(Creator))(entity);
				},
				[](SceneT* scene, Entity entity) {
					(scene->*static_cast<void (SceneT::*)(Entity)>(Destroyer))(entity);
				}
			};
			sceneInfo->mComponents.insert(uid, &componentInfo);

			mMetaInfo = Reflection::Reflect<ComponentT>(uid.GetHash());
			RegisterComponentType(uid.GetString());

			return *this;
		}

		SceneFactory<SceneT> EndComponent()
		{
			return SceneFactory<SceneT>();
		}

		template<auto Var>
		ComponentFactory& AddVar(const StringID& uid) noexcept
		{
			mMetaInfo.AddVar<Var>(uid.GetHash());
			return *this;
		}

		template<auto Func>
		ComponentFactory& AddFunc(const StringID& uid) noexcept
		{
			mMetaInfo.AddFunc<Func>(uid.GetHash());
			return *this;
		}
	};

	template<typename SceneT>
	class SceneFactory
	{
	public:
		SceneFactory& BeginScene(const StringID& uid)
		{
			Impl::SceneInfo<SceneT>* sceneInfo = Impl::InfoFactory<SceneT>::Resolve();
			sceneInfo->mIdentifier = uid;
			return *this;
		}

		template<typename ComponentT, auto Creator, auto Destroyer>
		ComponentFactory<ComponentT, SceneT> BeginComponent(const StringID& uid)
		{
			return ComponentFactory<ComponentT, SceneT>().BeginComponent<Creator, Destroyer>(uid);
		}

		SceneFactory& AddFunction(const StringID& uid)
		{
			return *this;
		}
	};

	template<typename SceneT>
	SceneFactory<SceneT> SceneReflect(const StringID& uid)
	{
		return SceneFactory<SceneT>().BeginScene(uid);
	}
}

#define CJING_BEGIN_SCENE(type, name)\
	do { \
		using ReflScene = type;     \
		ECS::SceneReflection::SceneReflect<type>(StringID(name))
#define CJING_BEGIN_COMPONENT(type) .BeginComponent<type, &ReflScene::Create##type, &ReflScene::Destroy##type>(StringID(#type))
#define CJING_ADD_VAR(type, name) .AddVar<type>(StringID(name))
#define CJING_END_COMPONENT() .EndComponent()
#define CJING_END_SCENE(); 	} while(false);
}