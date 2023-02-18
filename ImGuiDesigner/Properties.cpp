#include "Properties.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

void PropertyLabel(const char* lbl)
{
	ImGui::TableNextColumn();
	ImGui::Text(lbl);
	ImGui::TableNextColumn();
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
		ImGui::BeginTable("ProprtiesTable", 2, ImGuiTableFlags_SizingFixedFit);
		PropertyLabel("Label:");
		ImGui::PushItemWidth(120);
		ImGui::InputText("##property_label", &active_element->v_label);
		ImGui::PopItemWidth();
		PropertyLabel("Size:");
		ImGui::PushItemWidth(120);
		ImGui::InputFloat2("##property_size", (float*)&active_element->v_size);
		ImGui::PopItemWidth();
		PropertyLabel("Position:");
		ImGui::PushItemWidth(120);
		ImGui::InputFloat2("##property_pos", (float*)&active_element->v_pos);
		ImGui::PopItemWidth();
		PropertyLabel("Foreground:");
		ImGui::ColorEdit4("##property_foreground", (float*)&active_element->v_foreground, ImGuiColorEditFlags_NoInputs);
		PropertyLabel("Background:");
		ImGui::ColorEdit4("##property_background", (float*)&active_element->v_background, ImGuiColorEditFlags_NoInputs);
		active_element->RenderPropertiesInternal();
		ImGui::EndTable();
		if (ImGui::Button("Delete##property_delete"))
			active_element->Delete();
	}

	
	ImGui::End();
}