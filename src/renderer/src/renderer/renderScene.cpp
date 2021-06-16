
#include "renderScene.h"
#include "core\scene\reflection.h"

namespace Cjing3D
{	
    /// ////////////////////////////////////////////////////////////////////////////////
    /// RenderScene Systems

    MaterialSystem::MaterialSystem() {
        DeclareDependencies<TransformSystem, HierarchySystem>();
    }
    void MaterialSystem::Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)
    {
    }

    MeshSystem::MeshSystem() {
        DeclareDependencies<TransformSystem, HierarchySystem>();
    }
    void MeshSystem::Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)
    {
    }

    ObjectSystem::ObjectSystem() {
        DeclareDependencies<TransformSystem, HierarchySystem>();
    }
    void ObjectSystem::Update(Universe& universe, JobSystem::JobHandle& jobHandle, bool& waitJobs)
    {
    }

    /// ////////////////////////////////////////////////////////////////////////////////
    /// RenderScene Systems
    class RenderSceneImpl
    {
    public:
    };

    /// ////////////////////////////////////////////////////////////////////////////////
    /// RenderScene
    RenderScene::RenderScene(Engine& engine, Universe& universe) :
        mEngine(engine),
        mUniverse(universe)
    {
        // components
        mObjects = universe.RegisterComponents<ObjectComponent>(ECS::SceneReflection::RegisterComponentType("Object"));
        mObjectAABBs = universe.RegisterComponents<AABB>(ECS::SceneReflection::RegisterComponentType("ObjectAABB"));

        // systems
        universe.RegisterSystem(CJING_NEW(ObjectSystem));

        // reflection
        RenderScene::RegisterReflect();
    }

    RenderScene::~RenderScene()
    {
    }

    void RenderScene::Initialize()
    {
    }

    void RenderScene::Uninitialize()
    {
    }

    void RenderScene::Update(F32 dt)
    {
    }

    void RenderScene::LateUpdate(F32 dt)
    {
    }

    void RenderScene::Clear()
    {
    }

    Universe& RenderScene::GetUniverse()
    {
        return mUniverse;
    }

    void RenderScene::GetCullingResult(Visibility& cullingResult, Frustum& frustum, I32 cullingFlag)
    {
        struct CullingEntityList
        {
            U32 mCount = 0;
            U32* mList = nullptr;
        };

        JobSystem::JobHandle jobHandle = JobSystem::INVALID_HANDLE;

        if (cullingFlag & CULLING_FLAG_OBJECTS)
        {
            I32 count = mObjectAABBs->GetCount();
            cullingResult.mCulledObjects.resize(count);

            JobSystem::RunJobs(count, 64, [&](I32 jobIndex, JobSystem::JobGroupArgs* args, void* sharedMem) {
               
                CullingEntityList* list = (CullingEntityList*)sharedMem;
                if (args->isFirstJobInGroup_) {
                    list->mCount = 0;
                }

                ECS::Entity entity = mObjectAABBs->GetEntityByIndex(jobIndex);
                if (entity == ECS::INVALID_ENTITY) {
                    return false;
                }

                const AABB* aabb = mObjectAABBs->GetComponentByIndex(jobIndex);
                if (aabb != nullptr) 
                {
                    if (frustum.Overlaps(*aabb)) {
                        list->mList[list->mCount++] = jobIndex;
                    }
                }

                if (args->isLastJobInGroup_ && list->mCount > 0)
                {
                    I32 newCount = Concurrency::AtomicAdd(&cullingResult.mObjectCount, list->mCount);
                    for (int i = 0; i < list->mCount; i++) {
                        cullingResult.mCulledObjects[newCount - list->mCount + i] = list->mList[i];
                    }
                }

                return false;

            }, sizeof(CullingEntityList), &jobHandle);
        }

        if (jobHandle != JobSystem::INVALID_HANDLE) {
            JobSystem::Wait(&jobHandle);
        }
    }

    void RenderScene::RegisterReflect()
    {
        CJING_BEGIN_SCENE(RenderSceneImpl, "RenderScene")
        CJING_END_SCENE();
    }
}