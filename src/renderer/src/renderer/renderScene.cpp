
#include "renderScene.h"
#include "core\scene\reflection.h"

namespace Cjing3D
{	
    /// ////////////////////////////////////////////////////////////////////////////////
    /// RenderScene Impl

    class TestComponent
    {
    public:
        int mTestVal = 10;
    };

    class RenderSceneImpl
    {
    public:
        Engine& mEngine;
        Universe& mUniverse;
        int mTestVal;

    public:
        RenderSceneImpl(Engine& engine, Universe& universe) :
            mEngine(engine),
            mUniverse(universe)
        {
        }

        ~RenderSceneImpl()
        {
        }

        void CreateTestComponent(ECS::Entity entity) {
        }

        void DestroyTestComponent(ECS::Entity entity) {
        }
    };

    /// ////////////////////////////////////////////////////////////////////////////////
    /// RenderScene
    RenderScene::RenderScene(Engine& engine, Universe& universe)
    {
        mImpl = CJING_NEW(RenderSceneImpl)(engine, universe);

        RenderScene::RegisterReflect();
    }

    RenderScene::~RenderScene()
    {
        CJING_SAFE_DELETE(mImpl);
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
        return mImpl->mUniverse;
    }

    CullResult RenderScene::GetCullResult()
    {
        return CullResult();
    }

    void RenderScene::RegisterReflect()
    {
        CJING_BEGIN_SCENE(RenderSceneImpl, "RenderScene")
        CJING_END_SCENE();
    }
}