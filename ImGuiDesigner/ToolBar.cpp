#include "ToolBar.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "Workspace.h"
#include "Properties.h"
#include "ImGuiDesigner.h"
#include "../json/single_include/nlohmann/json.hpp"
#include <fstream>
#include "igd_elements.h"

template<typename T>
bool ToolBar::Tool(std::string name, float width)
{
	T ref_element;
	ImGuiElement* ref = (ImGuiElement*)&ref_element;
	if (ref->v_parent_required_id && !igd::active_workspace->active_element)
		return false;
	else if (ref->v_parent_required_id && !(ref->v_parent_required_id & igd::active_workspace->active_element->v_type_id))
		return false;

	if (!igd::active_workspace->active_element || !igd::active_workspace->active_element->v_element_filter || (igd::active_workspace->active_element->v_element_filter && (igd::active_workspace->active_element->v_element_filter & ref->v_type_id)))
		if (ImGui::Button(name.c_str(), { width, 0 }))
		{
			igd::active_workspace->AddNewElement((ImGuiElement*)(new T()));
			return true;
		}
	return false;
}

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

	igd::push_designer_theme();
	
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
	
	
	float width = 140;
	
	this->Tool<igd::ChildWindow>("Child Window", width);
	this->Tool<igd::Button>("Button", width);
	this->Tool<igd::InputText>("Input Text", width);
	this->Tool<igd::InputInt>("Input Int", width);
	this->Tool<igd::SliderInt>("Slider Int", width);
	this->Tool<igd::InputFloat>("Input Float", width);
	this->Tool<igd::SliderFloat>("Slider Float", width);
	this->Tool<igd::CheckBox>("Checkbox", width);
	this->Tool<igd::Text>("Text", width);
	this->Tool<igd::Separator>("Separator", width);
	this->Tool<igd::Combo>("Combo", width);
	this->Tool<igd::Selectable>("Selectable", width);
	this->Tool<igd::TabBar>("TabBar", width);
	this->Tool<igd::TabItem>("TabItem", width);
	
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
					igd::active_workspace->load(p.path());
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
	igd::pop_designer_theme();
}