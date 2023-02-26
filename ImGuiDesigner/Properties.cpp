#include "Properties.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
void Properties::PropertyLabel(const char* lbl)
{
	ImGui::TableNextColumn();
	ImGui::Text(lbl);
	ImGui::TableNextColumn();
}


void Properties::getChildParents(ImGuiElement* parent)
{
	for (auto& element : parent->children)
	{
		if (element == active_element || !element->v_can_have_children || element->delete_me)
			continue;
		if (element->children.size() > 0)
			getChildParents(element);
		if (ImGui::Selectable(element->v_id.c_str()))
		{
			active_element->PushUndo();
			active_element->v_parent = element;
		}
	}
}


void Properties::getAllChildren(ImGuiElement* parent)
{
	for (auto& element : parent->children)
	{
		if (element == active_element || element->delete_me)
			continue;
		if (element->children.size() > 0)
			getAllChildren(element);
		if (ImGui::Selectable(element->v_id.c_str()))
			active_element = element;
	}
}

void Properties::buildTree(ImGuiElement* parent)
{
	ImGuiContext& g = *GImGui;
	for (auto& element : parent->children)
	{
		if (element->delete_me)
			continue;
		if (!element->v_can_have_children && ImGui::Selectable(element->v_id.c_str()))
			active_element = element;

		if (element->v_can_have_children)
		{
			if (ImGui::TreeNode(element->v_id.c_str()))
			{
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					active_element = element;
				buildTree(element);
				ImGui::TreePop();
			}
		}
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
	if (ImGui::TreeNode("Element Tree"))
	{
		for (auto& e : igd::active_workspace->elements)
		{
			if (e->delete_me)
				continue;

			if (e->v_can_have_children)
			{
				if (ImGui::TreeNode(e->v_id.c_str()))
				{
					if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						active_element = e;
					buildTree(e);
					ImGui::TreePop();
				}
			}
			else
				if (ImGui::Selectable(e->v_id.c_str()))
					active_element = e;
		}
		ImGui::TreePop();
	}

	int item_width = 200;
	if (!active_element)
		active_element = igd::active_workspace->basic_workspace_element;
	bool is_workspace = active_element == igd::active_workspace->basic_workspace_element;
	if (active_element)
	{
		if (is_workspace)
		{
			ImGui::Text("Workspace properties");
			ImGui::Separator();
		}
		ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_SizingFixedFit);
		if (!is_workspace)
		{
			PropertyLabel("ID:");
			ImGui::PushItemWidth(item_width);
			ImGui::InputText("##property_id", &active_element->v_id);
		}
		if (active_element->v_property_flags & property_flags::label && !is_workspace)
		{
			PropertyLabel("Label:");
			ImGui::PushItemWidth(item_width);
			if (ImGui::InputText("##property_label", &active_element->v_label))
			{
				active_element->PushUndo();
			}
		}
		
		if (!is_workspace)
		{
			PropertyLabel("Size:");
			ImGui::PushItemWidth(item_width);
			if (ImGui::InputFloat2("##property_size", (float*)&active_element->v_size))
			{
				active_element->PushUndo();
			}
		}

		if (!is_workspace)
		{
			PropertyLabel("Position:");
			ImGui::PushItemWidth(item_width);
			if (ImGui::InputFloat2("##property_pos", (float*)&active_element->v_pos))
			{
				active_element->PushUndo();
			}
		}

		if (active_element->v_property_flags & property_flags::color_foreground)
		{
			PropertyLabel("Foreground:");
			if (ImGui::ColorEdit4("##property_foreground", (float*)&active_element->v_foreground, ImGuiColorEditFlags_NoInputs))
			{
				modified = true;
			}
		}
		if (active_element->v_property_flags & property_flags::color_background)
		{
			PropertyLabel("Background:");
			if (ImGui::ColorEdit4("##property_background", (float*)&active_element->v_background, ImGuiColorEditFlags_NoInputs))
			{
				modified = true;
			}
		}
		if (active_element->v_property_flags & property_flags::color_background_hovered)
		{
			PropertyLabel("Hovered:");
			if (ImGui::ColorEdit4("##property_background_hovered", (float*)&active_element->v_background_hovered, ImGuiColorEditFlags_NoInputs))
			{
				modified = true;
			}
		}
		if (active_element->v_property_flags & property_flags::color_background_active)
		{
			PropertyLabel("Active:");
			if (ImGui::ColorEdit4("##property_background_active", (float*)&active_element->v_background_active, ImGuiColorEditFlags_NoInputs)) 
			{
				modified = true;
			}
		}

		if (modified && !ImGui::IsPopupOpen("picker", ImGuiPopupFlags_AnyPopup))
		{
			modified = false;
			active_element->PushUndo();
		}

		if (active_element->v_property_flags & property_flags::border)
		{
			PropertyLabel("Border:");
			if (ImGui::Checkbox("##property_border", &active_element->v_border))
			{
				active_element->PushUndo();
			}
		}
		if (active_element->v_property_flags & property_flags::disabled && !is_workspace)
		{
			PropertyLabel("Disabled:");
			if (ImGui::Checkbox("##property_disabled", &active_element->v_disabled))
			{
				active_element->PushUndo();
			}
		}

		if (!is_workspace)
		{
			PropertyLabel("Parent:");
			ImGui::PushItemWidth(item_width);
			if (ImGui::BeginCombo("##property_parent", active_element->v_parent ? active_element->v_parent->v_id.c_str() : "None"))
			{
				if (ImGui::Selectable("None"))
				{
					active_element->v_parent = nullptr;
					active_element->PushUndo();
				}
				for (auto& element : igd::active_workspace->elements)
				{
					if (element == active_element || !element->v_can_have_children || element->delete_me)
						continue;
					if (element->children.size()>0)
						getChildParents(element);
					if (ImGui::Selectable(element->v_id.c_str()))
					{
						active_element->v_parent = element;
						active_element->PushUndo();
					}
				}
				ImGui::EndCombo();
			}
		}

		ImGui::PushItemWidth(item_width);
		active_element->RenderPropertiesInternal();
		
		if (active_element->v_can_have_children && active_element->children.size() > 0)
		{
			PropertyLabel("Widget:");
			if (ImGui::Button("Save##Json_Save"))
				active_element->SaveAsWidget(active_element->v_id);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("A widget will be saved comprised of all the children of this element.\nThe name will be the id of this element.");
			ImGui::EndTooltip();
		}
		
		ImGui::EndTable();
		if (ImGui::Button("Delete##property_delete"))
			active_element->Delete();
	}

	
	ImGui::End();
}