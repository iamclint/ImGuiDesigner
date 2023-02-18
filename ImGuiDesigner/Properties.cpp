#include "Properties.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiElement.h"
#include "Workspace.h"
void PropertyLabel(const char* lbl)
{
	ImGui::TableNextColumn();
	ImGui::Text(lbl);
	ImGui::TableNextColumn();
}


void Properties::getChildParents(ImGuiElement* parent)
{
	for (auto& element : parent->children)
	{
		if (element == active_element || !element->v_can_have_children)
			continue;
		if (element->children.size() > 0)
			getChildParents(element);
		if (ImGui::Selectable(element->v_id.c_str()))
			active_element->v_parent = element;
	}
}

void Properties::OnUIRender() {
	static char* buf = new char[25];
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "Input");
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGui::Begin("Properties");
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

	/*int v_flags;
	ImVec2 v_size;
	ImVec2 v_pos;
	std::string v_id;
	std::string v_label;
	ImColor v_foreground;
	ImColor v_background;
	int border;*/
	if (active_element)
	{
		ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_SizingFixedFit);
		PropertyLabel("ID:");
		ImGui::PushItemWidth(260);
		ImGui::InputText("##property_id", &active_element->v_id);
		PropertyLabel("Label:");
		ImGui::PushItemWidth(260);
		ImGui::InputText("##property_label", &active_element->v_label);
		PropertyLabel("Size:");
		ImGui::PushItemWidth(260);
		ImGui::InputFloat2("##property_size", (float*)&active_element->v_size);
		PropertyLabel("Position:");
		ImGui::PushItemWidth(260);
		ImGui::InputFloat2("##property_pos", (float*)&active_element->v_pos);
		PropertyLabel("Foreground:");
		ImGui::ColorEdit4("##property_foreground", (float*)&active_element->v_foreground, ImGuiColorEditFlags_NoInputs);
		PropertyLabel("Background:");
		ImGui::ColorEdit4("##property_background", (float*)&active_element->v_background, ImGuiColorEditFlags_NoInputs);
		PropertyLabel("Border:");
		ImGui::Checkbox("##property_border", &active_element->v_border);
		PropertyLabel("Parent:");
		ImGui::PushItemWidth(260);
		if (ImGui::BeginCombo("##property_parent", active_element->v_parent ? active_element->v_parent->v_id.c_str() : "None"))
		{
			if (ImGui::Selectable("None"))
			{
				active_element->v_parent = nullptr;
			}
			for (auto& element : igd::work->elements)
			{
				if (element == active_element || !element->v_can_have_children)
					continue;
				if (element->children.size()>0)
					getChildParents(element);
				if (ImGui::Selectable(element->v_id.c_str()))
					active_element->v_parent = element;
			}
			ImGui::EndCombo();
		}
		
		active_element->RenderPropertiesInternal();
		
		ImGui::EndTable();
		if (ImGui::Button("Delete##property_delete"))
			active_element->Delete();
	}

	
	ImGui::End();
}