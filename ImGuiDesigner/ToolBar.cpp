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

void ToolBar::OnUIRender() {
	static char* buf =new char[25];
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "Input");
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGui::Begin("ToolBar");
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	ImGui::PushItemWidth(60);
	if (ImGui::Button("New Workspace##toolbar_new_workspace"))
	{
		igd::properties->active_element = nullptr;
		igd::add_workspace = true;
	}

	if (ImGui::Button("Child Window##toolbar_input_child"))
	{
		AddNewElement((ImGuiElement*)(new igd::ChildWindow()));
	}
	if (ImGui::Button("Button##toolbar_input_button"))
	{
		AddNewElement((ImGuiElement*)(new igd::Button()));
	}
	ImGui::InputText("##toolbar_input_text", buf, 25, ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Custom Widgets");
	ImGui::Separator();

	//iterate all files in widgets folder
	for (auto& p : std::filesystem::directory_iterator("widgets"))
	{
		if (p.path().extension() == ".igd")
		{
			if (ImGui::Button(p.path().filename().stem().string().c_str()))
			{
				//load the file
				std::ifstream i(p.path().string());
				nlohmann::json j;
				try
				{
					j = nlohmann::json::parse(i);

				ImGuiElement* parent = nullptr;
				//check if first element is a child 
				if (j["elements"].at(0)["type"] == "child window")
				{
					std::cout << "Child Window found" << std::endl;
					igd::ChildWindow* b = new igd::ChildWindow();
					b->FromJSON(j["elements"].at(0));
					parent = (ImGuiElement*)b;
					AddNewElement((ImGuiElement*)b);
				}
				else
				{
					return;
				}
				for (int i = 0; auto& e : j["elements"])
				{	
					//skip the first element since it is already added
					if (i == 0)
					{
						i++;
						continue;
					}
					if (e["type"] == "button")
					{
						std::cout << "Adding a button" << std::endl;
						igd::Button* b = new igd::Button();
						ImGuiElement* f = (ImGuiElement*)b;
						f->v_parent = parent;
						b->FromJSON(e);
						parent->children.push_back((ImGuiElement*)b);
					}
					i++;
				}
				}
				catch (nlohmann::json::parse_error& ex)
				{
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