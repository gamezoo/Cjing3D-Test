#include "gameView.h"
#include "gpu\gpu.h"
#include "imguiRhi\imguiEx.h"
#include "renderer\renderer.h"
#include "renderer\renderScene.h"
#include "renderer\renderPath\renderGraphPath3D.h"
#include "renderer\renderImage.h"
#include "renderer\textureHelper.h"

namespace Cjing3D
{
	class EditorWidgetGameViewImpl
	{
	public:
		UniquePtr<RenderPath> mRenderPath = nullptr;
		Texture mGameTextrue;
		ImVec2 mPrevViewSize;

		void ResizeBuffers(const ImVec2& size)
		{
			mPrevViewSize = size;
			mGameTextrue.Clear();

			// texture 
			GPU::TextureDesc texDesc;
			texDesc.mWidth = (U32)size.x;
			texDesc.mHeight = (U32)size.y;
			texDesc.mFormat = GPU::GetBackBufferFormat();
			texDesc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;
			GPU::ResHandle res = GPU::CreateTexture(&texDesc, nullptr, "rtGameView");
			mGameTextrue.SetTexture(res, texDesc);
		}

		void Initialize()
		{
			mRenderPath = CJING_MAKE_UNIQUE<RenderGraphPath3D>();
			mRenderPath->Start();
		}

		void Uninitialize()
		{
			mRenderPath->Stop();
			mRenderPath.Reset();
		}

		void Update(F32 deltaTime)
		{
			if (!mRenderPath) return;

			RenderScene* scene = Renderer::GetRenderScene();
			if (!scene) {
				return;
			}

			ImVec2 viewPos = ImGui::GetCursorScreenPos();
			ImVec2 size = ImGui::GetContentRegionAvail();
			if (size.x <= 0 || size.y <= 0) {
				return;
			}

			if ((U32)mPrevViewSize.x != (U32)size.x || (U32)mPrevViewSize.y != (U32)size.y) {
				ResizeBuffers(size);
			}

			mRenderPath->Update(deltaTime);

			if (mGameTextrue.GetHandle())  {
				ImGui::Image((ImTextureID)mGameTextrue.GetHandlePtr(), size);
			}
			else {
				ImGuiEx::Rect(size.x, size.y, 0xffFF00FF);
			}
		}

		void Draw()
		{
			RenderScene* scene = Renderer::GetRenderScene();
			if (!scene) {
				return;
			}
			mRenderPath->Render();
			mRenderPath->Compose(mGameTextrue.GetHandle(), mGameTextrue.GetDesc());
		}
	};


	EditorWidgetGameView::EditorWidgetGameView(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "GameView";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavInputs;
		mImpl = CJING_NEW(EditorWidgetGameViewImpl);
	}

	EditorWidgetGameView::~EditorWidgetGameView()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void EditorWidgetGameView::Initialize()
	{
		mImpl->Initialize();
	}

	void EditorWidgetGameView::Uninitialize()
	{
		mImpl->Uninitialize();
	}

	void EditorWidgetGameView::Update(F32 deltaTime)
	{
		mImpl->Update(deltaTime);
	}

	void EditorWidgetGameView::Draw()
	{
		mImpl->Draw();
	}
}