#include "ToolBar.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "Workspace.h"
#include "Properties.h"
#include "igd_button.h"
#include "igd_childwindow.h"

void AddNewElement(ImGuiElement* ele)
{
	if (igd::properties->active_element && igd::properties->active_element->v_can_have_children)
	{
		igd::properties->active_element->children.push_back(ele);
		igd::properties->active_element->children.back()->v_parent = igd::properties->active_element;
	}
	else
	{
		igd::work->elements.push_back(ele);
		igd::properties->active_element = igd::work->elements.back();
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

	if (ImGui::IsAnyItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
	}
	ImGui::End();
}