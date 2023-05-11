#include "SettingsTabs.h"
#include "Settings.h"
#include "ImGuiDesigner.h"
#include "imgui.h"
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
namespace igd
{
	void Link(std::string url, std::string title="")
	{
		std::string caption = title.empty() ? url : title;
		ImColor color = ImColor(21, 199, 255, 255);
		ImVec2 size = ImGui::CalcTextSize(caption.c_str());
		ImVec2 pos = ImGui::GetWindowPos()+ImGui::GetCursorPos();
		ImGui::TextColored(color, caption.c_str());
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
	/*	if (ImGui::Checkbox("Relative sizing", &igd::settings->bools["relative_sizing"]))
			igd::settings->save();
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Relative sizing adjusts the size based on the remaining space available. For example, if two items each have a width of 50%%,");
			ImGui::Text("they would only take up 75%% of the available space because there is only 50%% of the original space left after the first item. Therefore,");
			ImGui::Text("the second item's 50%% width would only be 25%% of the total original width.");
			ImGui::Text("So to have 2 rectangle fill the entire space the first one would be 50%% while the next one is 100%%");
			ImGui::EndTooltip();
		}*/
	}
	void About()
	{
		ImGui::BeginTable("##about", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit);
		ImGui::TableNextColumn();	ImGui::Text("Main Gui"); ImGui::TableNextColumn();	ImGui::Text("Dear ImGui Docking Branch"); ImGui::TableNextColumn(); Link("https://github.com/ocornut/imgui/tree/docking", "GitHub");
		ImGui::TableNextColumn();	ImGui::Text("Application framework");	ImGui::TableNextColumn();	ImGui::Text("Walnut by The Cherno"); ImGui::TableNextColumn(); Link("https://github.com/TheCherno/Walnut", "GitHub");
		ImGui::TableNextColumn();	ImGui::Text("Application framework");	ImGui::TableNextColumn();	ImGui::Text("My fork of Walnut"); ImGui::TableNextColumn(); Link("https://github.com/iamclint/Walnut", "GitHub");
		ImGui::TableNextColumn();	ImGui::Text("Icons");	ImGui::TableNextColumn();	ImGui::Text("Icons 8"); ImGui::TableNextColumn(); Link("https://icons8.com/", "Website");
		ImGui::EndTable();
	
		ImGui::TextWrapped("I would like to extend a sincere thank you to everyone who has contributed their time and expertise to help shape ImGuiDesigner. Your valuable feedback and ideas have been essential to the development process, and I couldn't have done it without you.");
		ImGui::TextWrapped("Special thanks to"); ImGui::SameLine(); Link("https://github.com/veteri", "Zeez,"); ImGui::SameLine(); Link("https://github.com/TerryS94", "Terry,");  ImGui::SameLine(); Link("https://github.com/imgemmy", "J$k"); ImGui::SameLine(); ImGui::Text("and Joe for their exceptional contributions"); 
		ImGui::TextWrapped("and tireless efforts in testing and providing feedback.Their input has been instrumental in identifying issues and improving the overall user experience.");
		ImGui::TextWrapped("I'm grateful to have such a dedicated and supportive community, and I look forward to continuing to work together to make ImGuiDesigner the best it can be.");
		ImGui::TextWrapped("Thank you all again for your invaluable contributions.");

		
	}
	std::unordered_map<const char*, std::function<void()>> settings_tabs =
	{
		//{"Test", Test},
		{"About", About},
		{"Interactions", Interactions},

	};
}