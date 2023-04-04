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
	ImGuiElement* active = igd::active_workspace->active_element;
	if (ref->v_parent_required_id && !active)
		return false;
	else if (ref->v_parent_required_id && !(ref->v_parent_required_id & active->v_type_id))
		return false;

	if (!active || !active->v_element_filter || (active->v_element_filter && (active->v_element_filter & ref->v_type_id)))
		if (ImGui::Button(name.c_str(), { width, 0 }))
		{
			if (ref->v_type_id == (int)element_type::texture)
			{
				igd::notifications->OpenFile([ref](std::string file) {
					igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::Texture(file)), false, ref->v_auto_select);
				}, "*.png\0*.jpg\0All Files(*.*)\0 * .*\0");
			}
			else
			{
				igd::active_workspace->AddNewElement((ImGuiElement*)(new T()), false, ref->v_auto_select);
			}
			return true;
		}
	return false;
}
void ToolBar::RenderElements()
{


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
	this->Tool<igd::Texture>("Texture", width);
}
void ToolBar::RenderCustomWidgets()
{

	//iterate all files in widgets folder
	for (auto& p : std::filesystem::directory_iterator(igd::startup_path.string() + "/widgets"))
	{
		if (p.path().extension() == ".wgd")
		{
			if (ImGui::Button(p.path().filename().stem().string().c_str(), { 140, 0 }))
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
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	ImGui::Text("Interaction Mode");
	ImGui::Separator();
	ImGui::PushItemWidth(140);
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
	ImGui::BeginTabBar("##Toolbar_Tabs");
	if (ImGui::BeginTabItem("Elements"))
	{
		this->RenderElements();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Custom Widgets"))
	{
		this->RenderCustomWidgets();
		ImGui::EndTabItem();
	}
	
	if (ImGui::IsAnyItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
	}
	ImGui::EndTabBar();
	ImGui::End();
	igd::pop_designer_theme();
}