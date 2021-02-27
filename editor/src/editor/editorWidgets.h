#pragma once

#include "core\common\common.h"
#include "core\container\dynamicArray.h"
#include "core\string\string.h"
#include "core\concurrency\concurrency.h"
#include "core\filesystem\path.h"
#include "imguiRhi\manager.h"

namespace Cjing3D
{
	class GameEditor;

	class EditorWidget
	{
	public:
		EditorWidget(GameEditor& editor);
		virtual ~EditorWidget();

		virtual void Initialize() {}
		virtual void Update(F32 deltaTime) {}
		virtual void Uninitialize() {}

		bool Begin();
		void End();

		bool IsVisible()const { return mIsVisible; }
		void SetVisible(bool isVisible) { mIsVisible = isVisible; }
		F32  GetHeight()const { return mHeight; }

	protected:
		virtual bool PreBegin() { return true; }
		virtual bool PostBegin() { return true; }

	protected:
		GameEditor& mEditor;

		bool mIsVisible = true;
		bool mIsBegun = false;
		bool mIsWindow = true;
		I32 mWidgetFlags = 0;
		F32x2 mPos = F32x2(-1.0f, -1.0f);
		F32x2 mSize = F32x2(-1.0f, -1.0f);
		F32 mHeight = 0.0f;
		std::string mTitleName;
		ImGuiWindow* mWindow = nullptr;
		U32 mPushedStyleVar = 0;
	};

	class EditorWidgetEntityInspector : public EditorWidget
	{
	public:
		EditorWidgetEntityInspector(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};

	class EditorWidgetEntityList : public EditorWidget
	{
	public:
		EditorWidgetEntityList(GameEditor& editor);
		void Update(F32 deltaTime)override;
	};
}