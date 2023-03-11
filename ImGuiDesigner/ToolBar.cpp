#include "ToolBar.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "Workspace.h"
#include "Properties.h"
#include "igd_elements.h"
#include "ImGuiDesigner.h"
#include "../json/single_include/nlohmann/json.hpp"
#include <fstream>

void ToolBar::OnUIRender() {


	if (igd::active_workspace)
	{
		igd::active_workspace->RenderCode();
		igd::active_workspace->RenderAdd();
	}
	else
		return;
	
	static char* buf =new char[25];
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "Input Text");
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGui::Begin("ToolBar");
	ImGui::Text("Interaction Mode");
	ImGui::Separator();
	if (ImGui::BeginCombo("##", igd::active_workspace->interaction_mode == InteractionMode::designer ? "Designer" : "User"))
	{
		if (ImGui::Selectable("Designer"))
		{
			igd::active_workspace->interaction_mode = InteractionMode::designer;
		}
		if (ImGui::Selectable("User"))
		{
			igd::active_workspace->interaction_mode = InteractionMode::user;
		}
		ImGui::EndCombo();
	}
	ImGui::Spacing(); ImGui::Spacing();
	ImGui::Spacing(); ImGui::Spacing();

	ImGui::Text("Elements");
	ImGui::Separator();
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	
	if (ImGui::Button("Child Window##toolbar_input_child", {140, 0}))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::ChildWindow()));
	}
	if (ImGui::Button("Button##toolbar_input_button", { 140, 0 }))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::Button()));
	}
	ImGui::PushItemWidth(140);
	ImGui::InputText("##toolbar_input_text", buf, 25, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::InputText()));
	}
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "1.67456");
	ImGui::PushItemWidth(140);
	ImGui::InputText("##toolbar_input_float", buf, 25, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::InputFloat()));
	}
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "32");
	ImGui::PushItemWidth(140);
	ImGui::InputText("##toolbar_input_int", buf, 25, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::InputInt()));
	}
	ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
	ImGui::BeginDisabled();
	static float slider_float = 13.426f;
	ImGui::SliderFloat("##toolbar_input_slider_float", &slider_float, 0, 100);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		std::cout << "wtf" << std::endl;
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::SliderFloat()));
	}
	static int slider_int = 13;
	ImGui::SliderInt("##toolbar_input_slider_int", &slider_int, 8.0, 72);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::SliderInt()));
	}
	static bool check_bool = true;
	ImGui::Checkbox("##toolbar_input_checkbox", &check_bool);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::CheckBox()));
	}

	ImGui::EndDisabled();
	ImGui::Text("Basic text");
	if (ImGui::IsItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::Text()));
	}
	

	ImGui::Button("Separator");
	if (ImGui::IsItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::Separator()));
	}

	ImGui::PopStyleVar();
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Custom Widgets");
	ImGui::Separator();

	
	//iterate all files in widgets folder
	for (auto& p : std::filesystem::directory_iterator(igd::startup_path.string() + "/widgets"))
	{
		if (p.path().extension() == ".wgd")
		{
			if (ImGui::Button(p.path().filename().stem().string().c_str(), {140, 0}))
			{
				//load the file
				igd::active_workspace->load(p.path());
			}
			if (ImGui::BeginPopupContextItem(p.path().filename().string().c_str()))
			{
				if (ImGui::MenuItem("Add"))
				{

				}
				if (ImGui::MenuItem("Delete"))
				{
					igd::notifications->Confirmation("Delete", "Are you sure you wish to delete " + p.path().filename().string(), "", [p](bool conf) {
						if (conf)
						{
							std::filesystem::remove(p.path());
						}
						});
				}
				ImGui::EndPopup();
			}
		}

	}
	
	if (ImGui::IsAnyItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
	}
	
	ImGui::End();
}