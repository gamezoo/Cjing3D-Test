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
		GPU::ResHandle mFrameBinding;
		ImVec2 mPrevViewSize;

		void ResizeBuffers(const ImVec2& size)
		{
			mPrevViewSize = size;
			mGameTextrue.Clear();
			GPU::DestroyResource(mFrameBinding);

			// texture 
			GPU::TextureDesc texDesc;
			texDesc.mWidth = (U32)size.x;
			texDesc.mHeight = (U32)size.y;
			texDesc.mFormat = GPU::GetBackBufferFormat();
			texDesc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;
			GPU::ResHandle res = GPU::CreateTexture(&texDesc, nullptr, "rtGameView");
			mGameTextrue.SetTexture(res, texDesc);

			// frame binding set
			GPU::FrameBindingSetDesc desc;
			desc.mAttachments.push(GPU::BindingFrameAttachment::RenderTarget(mGameTextrue.GetHandle()));
			mFrameBinding = GPU::CreateFrameBindingSet(&desc, "fbGameView");
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

			GPU::DestroyResource(mFrameBinding);
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

			GPU::CommandList* cmd = GPU::CreateCommandlist();
			if (auto binding = cmd->BindScopedFrameBindingSet(mFrameBinding))
			{
				cmd->EventBegin("GameCompose");
				mRenderPath->Compose(*cmd);
				cmd->EventEnd();
			}
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