#include "settings.h"
#include "core\serialization\jsonArchive.h"
#include "imguiRhi\manager.h"

namespace Cjing3D
{
	static const char DEFAULT_SETTINGS_PATH[] = "default_editor.ini";
	static const char SETTINGS_PATH[] = "editor.ini";
	static const U32  SETTING_MAGIC = 0x239F1232;
	static const I32  SETTING_MAJOR = 1;
	static const I32  SETTING_MINOR = 0;

	namespace 
	{
		void LoadImGuiStyle(JsonArchive& archive)
		{
			auto& style = ImGui::GetStyle();

			for (int i = 0; i < ImGuiCol_COUNT; ++i)
			{
				F32x4 colors;
				archive.Read(ImGui::GetStyleColorName(i), colors);
				style.Colors[i].x = colors.x();
				style.Colors[i].y = colors.y();
				style.Colors[i].z = colors.z();
				style.Colors[i].w = colors.w();
			}

#define READ_PROPERTY(name) archive.Read(#name, style.name)
#define READ_PROPERTY_VEC2(name) do { F32x2 val; archive.Read(#name, val); style.name.x = val.x(); style.name.y = val.y();  } while (false);		

			READ_PROPERTY(Alpha);
			READ_PROPERTY_VEC2(WindowPadding);
			READ_PROPERTY(WindowRounding);
			READ_PROPERTY(WindowBorderSize);
			READ_PROPERTY_VEC2(WindowMinSize);
			READ_PROPERTY_VEC2(WindowTitleAlign);
			READ_PROPERTY(ChildRounding);
			READ_PROPERTY(ChildBorderSize);
			READ_PROPERTY(PopupRounding);
			READ_PROPERTY(PopupBorderSize);
			READ_PROPERTY_VEC2(FramePadding);
			READ_PROPERTY(FrameRounding);
			READ_PROPERTY(FrameBorderSize);
			READ_PROPERTY_VEC2(ItemSpacing);
			READ_PROPERTY_VEC2(ItemInnerSpacing);
			READ_PROPERTY_VEC2(TouchExtraPadding);
			READ_PROPERTY(IndentSpacing);
			READ_PROPERTY(ColumnsMinSpacing);
			READ_PROPERTY(ScrollbarSize);
			READ_PROPERTY(ScrollbarRounding);
			READ_PROPERTY(GrabMinSize);
			READ_PROPERTY(GrabRounding);
			READ_PROPERTY(TabRounding);
			READ_PROPERTY(TabBorderSize);
			READ_PROPERTY_VEC2(ButtonTextAlign);
			READ_PROPERTY_VEC2(SelectableTextAlign);
			READ_PROPERTY_VEC2(DisplayWindowPadding);
			READ_PROPERTY_VEC2(DisplaySafeAreaPadding);
			READ_PROPERTY(MouseCursorScale);
			READ_PROPERTY(AntiAliasedLines);
			READ_PROPERTY(AntiAliasedFill);
			READ_PROPERTY(CurveTessellationTol);
			READ_PROPERTY(CircleSegmentMaxError);

#undef READ_PROPERTY
#undef READ_PROPERTY_VEC2
		}

		void SaveImGuiStyle(JsonArchive& archive)
		{
			auto& style = ImGui::GetStyle();

#define WRITE_PROPERTY(name) archive.Write(#name, style.name)
#define WRITE_PROPERTY_VEC2(name) do { F32x2 val = { style.name.x, style.name.y }; archive.Write(#name, val);  } while (false);		

			WRITE_PROPERTY(Alpha);
			WRITE_PROPERTY_VEC2(WindowPadding);
			WRITE_PROPERTY(WindowRounding);
			WRITE_PROPERTY(WindowBorderSize);
			WRITE_PROPERTY_VEC2(WindowMinSize);
			WRITE_PROPERTY_VEC2(WindowTitleAlign);
			WRITE_PROPERTY(ChildRounding);
			WRITE_PROPERTY(ChildBorderSize);
			WRITE_PROPERTY(PopupRounding);
			WRITE_PROPERTY(PopupBorderSize);
			WRITE_PROPERTY_VEC2(FramePadding);
			WRITE_PROPERTY(FrameRounding);
			WRITE_PROPERTY(FrameBorderSize);
			WRITE_PROPERTY_VEC2(ItemSpacing);
			WRITE_PROPERTY_VEC2(ItemInnerSpacing);
			WRITE_PROPERTY_VEC2(TouchExtraPadding);
			WRITE_PROPERTY(IndentSpacing);
			WRITE_PROPERTY(ColumnsMinSpacing);
			WRITE_PROPERTY(ScrollbarSize);
			WRITE_PROPERTY(ScrollbarRounding);
			WRITE_PROPERTY(GrabMinSize);
			WRITE_PROPERTY(GrabRounding);
			WRITE_PROPERTY(TabRounding);
			WRITE_PROPERTY(TabBorderSize);
			WRITE_PROPERTY_VEC2(ButtonTextAlign);
			WRITE_PROPERTY_VEC2(SelectableTextAlign);
			WRITE_PROPERTY_VEC2(DisplayWindowPadding);
			WRITE_PROPERTY_VEC2(DisplaySafeAreaPadding);
			WRITE_PROPERTY(MouseCursorScale);
			WRITE_PROPERTY(AntiAliasedLines);
			WRITE_PROPERTY(AntiAliasedFill);
			WRITE_PROPERTY(CurveTessellationTol);
			WRITE_PROPERTY(CircleSegmentMaxError);

#undef WRITE_PROPERTY
#undef WRITE_PROPERTY_VEC2

			for (int i = 0; i < ImGuiCol_COUNT; ++i)
			{
				F32x4 colors = {
					style.Colors[i].x,
					style.Colors[i].y,
					style.Colors[i].z,
					style.Colors[i].w
				};
				archive.Write(ImGui::GetStyleColorName(i), colors);
			}
		}
	}

	bool EditorSettings::Load(JsonArchive& archive)
	{
		if (!archive.OpenJson(SETTINGS_PATH))
		{
			if (!archive.OpenJson(DEFAULT_SETTINGS_PATH)) {
				return false;
			}
		}

		// check setting valid
		U32 magic = 0;
		archive.Read("magic", magic);
		if (magic != SETTING_MAGIC) {
			return false;
		}
		I32 major = 0;
		archive.Read("version_major", major);
		if (major != SETTING_MAJOR) {
			return false;
		}
		
		// imgui
		archive.Read("imgui_ini", mImGuiIni);
		archive.Read("imgui_style", [](JsonArchive& archive) {
			LoadImGuiStyle(archive);
		});
		return true;
	}

	void EditorSettings::Save(JsonArchive& archive)
	{
		archive.Write("magic", SETTING_MAGIC);
		archive.Write("version_major", SETTING_MAJOR);
		archive.Write("version_minor", SETTING_MINOR);
		archive.Write("imgui_ini", mImGuiIni);
		archive.WriteCallback("imgui_style", [](JsonArchive& archive) {
			SaveImGuiStyle(archive);
		});

		archive.Save(SETTINGS_PATH);
	}
}