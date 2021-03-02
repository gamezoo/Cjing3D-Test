#include "shaderEditor.h"
#include "imguiRhi\manager.h"
#include "core\platform\platform.h"

namespace Cjing3D
{
	void ShaderEditor::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
		Path fullPath(context.GetFileSystem().GetBasePath());
		fullPath.AppendPath(res->GetPath());
		if (ImGui::Button("Open externally")) 
		{
			if (!Platform::ShellExecuteOpen(fullPath.c_str(), nullptr)) {
				Logger::Warning("Failed to open %s in exeternal editor.", fullPath.c_str());
			}
		}
	}
}