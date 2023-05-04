#include "SettingsTabs.h"
#include "Settings.h"
#include "ImGuiDesigner.h"
#include "imgui.h"
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
namespace igd
{
	void Link(std::string url)
	{
		ImColor color = ImColor(21, 199, 255, 255);
		ImVec2 size = ImGui::CalcTextSize(url.c_str());
		ImVec2 pos = ImGui::GetWindowPos()+ImGui::GetCursorPos();
		ImGui::TextColored(color, url.c_str());
		ImGui::GetWindowDrawList()->AddLine({ pos.x, pos.y + size.y + 2 }, { pos.x + size.x, pos.y + size.y + 2 }, color, 1.0f);
		if (ImGui::IsItemHovered())
		{
			GImGui->MouseCursor = ImGuiMouseCursor_Hand;
			if (ImGui::IsItemClicked())
				ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	void Test()
	{
		ImGui::Text("Test options");
	}
	void Interactions()
	{
		if (ImGui::Checkbox("Select all selects all children", &igd::settings->bools["select_children"]))
			igd::settings->save();
		if (ImGui::Checkbox("Select parent after copy", &igd::settings->bools["select_copy_parent"]))
			igd::settings->save();
	}
	void About()
	{
		ImGui::Text("Icons provided by"); 
		ImGui::SameLine(0, -1);
		Link("https://icons8.com/");
		
	}
	std::unordered_map<const char*, std::function<void()>> settings_tabs =
	{
		//{"Test", Test},
		{"About", About},
		{"Interactions", Interactions},

	};
}