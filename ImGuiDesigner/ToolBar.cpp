#include "ToolBar.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "Workspace.h"
#include "Properties.h"
#include "igd_elements.h"
#include "ImGuiDesigner.h"
#include "../json/single_include/nlohmann/json.hpp"
#include <fstream>

void AddNewElement(ImGuiElement* ele)
{
	if (igd::properties->active_element && igd::properties->active_element->v_can_have_children)
	{
		igd::properties->active_element->children.push_back(ele);
		igd::properties->active_element->children.back()->v_parent = igd::properties->active_element;
	}
	else
	{
		igd::properties->active_element = nullptr;
		igd::active_workspace->elements.push_back(ele);
		igd::properties->active_element = igd::active_workspace->elements.back();
	}

}

ImGuiElement* CreateElementFromJson(nlohmann::json& obj, ImGuiElement* parent)
{
	if (obj["type"] == "child window")
	{
		ImGuiElement* new_parent = nullptr;
		std::cout << "Child Window found" << std::endl;
		igd::ChildWindow* b = new igd::ChildWindow();
		b->FromJSON(obj);
		new_parent = (ImGuiElement*)b;
		if (parent)
			parent->children.push_back((ImGuiElement*)b);
		else
			AddNewElement((ImGuiElement*)b);
		new_parent->v_parent = parent;
		return new_parent;
	}
	else if (obj["type"] == "button")
	{
		std::cout << "Adding a button" << std::endl;
		igd::Button* b = new igd::Button();
		ImGuiElement* f = (ImGuiElement*)b;
		f->v_parent = parent;
		b->FromJSON(obj);
		parent->children.push_back((ImGuiElement*)b);
		return f;
	}
}


void GetAllChildren(nlohmann::json j, ImGuiElement* parent)
{
	ImGuiContext& g = *GImGui;
	std::cout << "j: " << j.dump() << std::endl;
	for (int i = 0; auto & e : j["children"])
	{
		std::cout << "e: " << e.dump() << std::endl;
		CreateElementFromJson(e, parent);
		if (e["children"].size() > 0)
		{
			GetAllChildren(e, parent->children[i]);
		}
		i++;
	}
}


void ToolBar::OnUIRender() {
	static char* buf =new char[25];
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "Input Text");
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGui::Begin("ToolBar");
	ImGui::PushItemWidth(140);
	if (ImGui::Button("New Workspace##toolbar_new_workspace"))
	{
		igd::properties->active_element = nullptr;
		igd::add_workspace = true;
	}
	ImGui::Text("Elements");
	ImGui::Separator();
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	
	if (ImGui::Button("Child Window##toolbar_input_child", {140, 0}))
	{
		AddNewElement((ImGuiElement*)(new igd::ChildWindow()));
	}
	if (ImGui::Button("Button##toolbar_input_button", { 140, 0 }))
	{
		AddNewElement((ImGuiElement*)(new igd::Button()));
	}
	ImGui::PushItemWidth(140);
	ImGui::InputText("##toolbar_input_text", buf, 25, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		
	}
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "1.67456");
	ImGui::PushItemWidth(140);
	ImGui::InputText("##toolbar_input_float", buf, 25, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{

	}
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "32");
	ImGui::PushItemWidth(140);
	ImGui::InputText("##toolbar_input_int", buf, 25, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{

	}
	ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
	ImGui::BeginDisabled();
	static float slider_float = 13.426;
	ImGui::SliderFloat("##toolbar_input_slider_float", &slider_float, 0, 100);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{

	}
	static int slider_int = 13;
	ImGui::SliderInt("##toolbar_input_slider_int", &slider_int, 8.0f, 72.0f, "%.0f");
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{

	}
	static bool check_bool = true;
	ImGui::Checkbox("##toolbar_input_checkbox", &check_bool);
	if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{

	}

	ImGui::EndDisabled();
	ImGui::Text("Basic text");
	if (ImGui::IsItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{

		}
	}
	
	ImGui::PopStyleVar();
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Custom Widgets");
	ImGui::Separator();
	
	//iterate all files in widgets folder
	for (auto& p : std::filesystem::directory_iterator("widgets"))
	{
		if (p.path().extension() == ".wgd")
		{
			if (ImGui::Button(p.path().filename().stem().string().c_str(), {140, 0}))
			{
				//load the file
				std::ifstream i(p.path().string());
				nlohmann::json j;
				try
				{
					j = nlohmann::json::parse(i);
					ImGuiElement* parent = CreateElementFromJson(j["child_window"], nullptr);
					GetAllChildren(j["child_window"], parent);
				}
				catch (nlohmann::json::parse_error& ex)
				{
					igd::notifications->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
					std::cerr << "parse error at byte " << ex.byte << std::endl << ex.what() << std::endl;
				}
			}
		}
	}


	if (ImGui::IsAnyItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
	}
	
	ImGui::End();
}