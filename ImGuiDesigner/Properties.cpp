#include "Properties.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <Windows.h>
#include "vulkan/vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
void Properties::PropertyLabel(const char* lbl)
{
	ImGui::TableNextColumn();
	ImGui::Text(lbl);
	ImGui::TableNextColumn();
}
void Properties::PropertySeparator()
{
	ImGui::TableNextColumn();
	ImGui::Separator();
	ImGui::TableNextColumn();
	ImGui::Separator();
}

void Properties::getChildParents(ImGuiElement* parent)
{
	for (auto& element : parent->children)
	{
		if (element == igd::active_workspace->active_element || !element->v_can_have_children || element->delete_me)
			continue;
		if (element->children.size() > 0)
			getChildParents(element);
		if (ImGui::Selectable(element->v_id.c_str()))
		{
			igd::active_workspace->active_element->PushUndo();
			igd::active_workspace->active_element->v_parent = element;
		}
	}
}


void Properties::getAllChildren(ImGuiElement* parent)
{
	for (auto& element : parent->children)
	{
		if (element == igd::active_workspace->active_element || element->delete_me)
			continue;
		if (element->children.size() > 0)
			getAllChildren(element);
		if (ImGui::Selectable(element->v_id.c_str()))
			igd::active_workspace->active_element = element;
	}
}

void Properties::buildTree(ImGuiElement* current_element)
{
	ImGuiElement* drop_target = nullptr;
	if (ImGui::GetDragDropPayload())
	{
		ImGuiElement* drag_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
		if (drag_element == current_element)
		{
			return;
		}
	}

	ImGuiContext& g = *GImGui;
	int flags = 0;
	if (current_element->children.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (igd::active_workspace->active_element == current_element)
		flags |= ImGuiTreeNodeFlags_Selected;
	bool IsOpen = ImGui::TreeNodeEx(current_element->v_id.c_str(), flags);
	{
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			igd::active_workspace->active_element = current_element;
		if (ImGui::BeginDragDropTarget()) {
			{
				ImGuiElement* source_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
				drop_target = current_element;
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
			{
				std::stringstream ss;
				ImGuiElement* source_element = *(ImGuiElement**)(payload->Data);


				bool parent_changed = false;
				//dragged onto an element that can have children just add it to the children
				if (current_element->v_can_have_children && source_element->v_parent != current_element)
				{
					parent_changed = true;
					source_element->v_parent = current_element;
					source_element->PushUndo();
				}
				else if (source_element->v_parent != current_element->v_parent && source_element->v_parent!=current_element)
				{
					parent_changed = true;
					source_element->v_parent = current_element->v_parent;
					source_element->PushUndo();
				}

				int new_index = current_element->v_render_index;
				if (current_element->v_render_index < source_element->v_render_index)
					new_index += 1;

				if (!parent_changed)
				{
					if (source_element->v_parent)
						igd::VecMove(source_element->v_parent->children, source_element->v_render_index, new_index);
					else
						igd::VecMove(igd::active_workspace->elements, source_element->v_render_index, new_index);
				}
				else
				{
					source_element->v_render_index = new_index;
					igd::active_workspace->sort_buffer.push_back(source_element);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginDragDropSource()) {
			intptr_t f = (intptr_t)current_element;
			ImGui::SetDragDropPayload("_TREENODE", &current_element, sizeof(current_element));
			//ImGui::Text(current_element->v_id.c_str());
			ImGui::EndDragDropSource();
		}

		if (IsOpen)
		{
			for (auto& element : current_element->children)
			{
				if (element->delete_me)
					continue;
				buildTree(element);
			}
			ImGui::TreePop();
		}

		if (current_element == drop_target && ImGui::GetDragDropPayload())
		{
			ImGuiElement* source_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
			ImGui::BeginDisabled();
			if (ImGui::TreeNodeEx(source_element->v_id.c_str(), ImGuiTreeNodeFlags_Leaf))
				ImGui::TreePop();
			ImGui::EndDisabled();
		}
	}
}


void Properties::OnUpdate(float f)
{

}


const char* getPropertyId(std::string name)
{
	return ("##property_" + name + "_" + igd::active_workspace->active_element->v_id).c_str();
}


void Properties::General()
{
	ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_SizingFixedFit);
	if (!is_workspace)
	{
		PropertyLabel("ID:");
		ImGui::PushItemWidth(item_width);
		ImGui::InputText("##property_id", &igd::active_workspace->active_element->v_id);
	}
	if (igd::active_workspace->active_element->v_property_flags & property_flags::label && !is_workspace)
	{
		PropertyLabel("Label:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::InputText(getPropertyId("label"), &igd::active_workspace->active_element->v_label))
		{
			igd::active_workspace->active_element->PushUndo();
		}
	}


	PropertyLabel("Font:");
	ImGui::PushItemWidth(item_width);
	std::filesystem::path font_path = igd::active_workspace->active_element->v_font.path;
	if (ImGui::BeginCombo(getPropertyId("font"), font_path.stem().string() == "" ? "Inherit" : font_path.stem().string().c_str()))
	{
		//create directory if doesn't exist
		std::filesystem::path fonts_dir = igd::startup_path.string() + "/fonts";
		if (!std::filesystem::exists(fonts_dir))
			std::filesystem::create_directory(fonts_dir);
		if (ImGui::Selectable("Inherit"))
		{
			igd::active_workspace->active_element->v_font.name = "";
			igd::active_workspace->active_element->v_font.font = nullptr;
			igd::active_workspace->active_element->PushUndo();
		}
		for (auto& p : std::filesystem::directory_iterator(igd::font_manager->GetWindowsFontsDirectory()))
		{
			if (p.path().extension() == ".ttf" && ImGui::Selectable(p.path().stem().string().c_str()))
				igd::font_manager->LoadFont(p.path(), igd::active_workspace->active_element->v_font.size, igd::active_workspace->active_element);
		}
		for (auto& p : std::filesystem::directory_iterator(fonts_dir))
		{
			if (p.path().extension() == ".ttf" && ImGui::Selectable(p.path().stem().string().c_str()))
				igd::font_manager->LoadFont(p.path(), igd::active_workspace->active_element->v_font.size, igd::active_workspace->active_element);
		}
		ImGui::EndCombo();
	}

	PropertyLabel("Font Size:");
	ImGui::PushItemWidth(item_width);
	if (ImGui::InputInt(getPropertyId("font_size"), &igd::active_workspace->active_element->v_font.size) && igd::active_workspace->active_element->v_font.size > 0 && igd::active_workspace->active_element->v_font.font)
		igd::font_manager->LoadFont(igd::active_workspace->active_element->v_font.path, igd::active_workspace->active_element->v_font.size, igd::active_workspace->active_element);

	if (!is_workspace)
	{
		PropertyLabel("Size:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::InputFloat2(getPropertyId("size"), (float*)&igd::active_workspace->active_element->v_size.value))
		{
			igd::active_workspace->active_element->PushUndo();
		}
		ImGui::SameLine();
		if (ImGui::BeginCombo(getPropertyId("size_type"), igd::active_workspace->active_element->v_size.type == Vec2Type::Absolute ? "Absolute" : "Relative (%)"))
		{
			if (ImGui::Selectable("Absolute"))
				igd::active_workspace->active_element->v_size.type = Vec2Type::Absolute;
			if (ImGui::Selectable("Relative (%)"))
			{
				igd::active_workspace->active_element->v_size.type = Vec2Type::Relative;

				if (igd::active_workspace->active_element->v_size.value.y == 0)
					igd::active_workspace->active_element->v_size.value.y = 100;
				if (igd::active_workspace->active_element->v_size.value.x == 0)
					igd::active_workspace->active_element->v_size.value.x = 100;

				igd::active_workspace->active_element->v_size.value.y = std::clamp(igd::active_workspace->active_element->v_size.value.y, 1.f, 100.f);
				igd::active_workspace->active_element->v_size.value.x = std::clamp(igd::active_workspace->active_element->v_size.value.x, 1.f, 100.f);
			}
			ImGui::EndCombo();
		}

	}

	if (igd::active_workspace->active_element->v_property_flags & property_flags::pos && !is_workspace)
	{
		PropertyLabel("Position:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::InputFloat2(getPropertyId("pos"), (float*)&igd::active_workspace->active_element->v_pos.value))
		{
			igd::active_workspace->active_element->PushUndo();
		}
		ImGui::SameLine();
		if (ImGui::BeginCombo(getPropertyId("pos_type"), igd::active_workspace->active_element->v_pos.type == Vec2Type::Absolute ? "Absolute" : "Relative (%)"))
		{
			if (ImGui::Selectable("Absolute"))
				igd::active_workspace->active_element->v_pos.type = Vec2Type::Absolute;
			if (ImGui::Selectable("Relative (%)"))
				igd::active_workspace->active_element->v_pos.type = Vec2Type::Relative;
			ImGui::EndCombo();
		}
	}

	if (!is_workspace)
	{
		PropertyLabel("Sameline:");
		ImGui::Checkbox("##property_sameline", &igd::active_workspace->active_element->v_sameline);
	}


	if (modified && !ImGui::IsPopupOpen("picker", ImGuiPopupFlags_AnyPopup))
	{
		modified = false;
		igd::active_workspace->active_element->PushUndo();
	}
	if (igd::active_workspace->active_element->v_property_flags & property_flags::disabled && !is_workspace)
	{
		PropertyLabel("Disabled:");
		if (ImGui::Checkbox("##property_disabled", &igd::active_workspace->active_element->v_disabled))
		{
			igd::active_workspace->active_element->PushUndo();
		}
	}

	if (!is_workspace)
	{
		PropertyLabel("Parent:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::BeginCombo(getPropertyId("parent"), igd::active_workspace->active_element->v_parent ? igd::active_workspace->active_element->v_parent->v_id.c_str() : "None"))
		{
			if (ImGui::Selectable("None"))
			{
				igd::active_workspace->active_element->v_parent = nullptr;
				igd::active_workspace->active_element->PushUndo();
			}
			for (auto& element : igd::active_workspace->elements)
			{
				if (element == igd::active_workspace->active_element || !element->v_can_have_children || element->delete_me)
					continue;
				if (element->children.size() > 0)
					getChildParents(element);
				if (ImGui::Selectable(element->v_id.c_str()))
				{
					igd::active_workspace->active_element->v_parent = element;
					igd::active_workspace->active_element->PushUndo();
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::PushItemWidth(item_width);
	igd::active_workspace->active_element->RenderPropertiesInternal();

	if (igd::active_workspace->active_element->v_can_have_children && igd::active_workspace->active_element->children.size() > 0)
	{
		PropertyLabel("Widget:");
		if (ImGui::Button("Save##Json_Save"))
			igd::active_workspace->active_element->SaveAsWidget(igd::active_workspace->active_element->v_id);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("A widget will be saved comprised of all the children of this element.\nThe name will be the id of this element.");
		ImGui::EndTooltip();
	}
	
	ImGui::EndTable();
}

void Properties::Colors()
{
	ImGui::BeginTable("PropertiesTableColors", 2, ImGuiTableFlags_SizingFixedFit);
	if (igd::active_workspace->active_element->v_colors.size() > 0)
	{
		//PropertyLabel("");
		//ImGui::Dummy({26, 0}); ImGui::SameLine();
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Inherit all Colors##inherit_colors", &igd::active_workspace->active_element->v_inherit_all_colors))
			modified = true;
		ImGui::TableNextColumn();
	}
	for (auto& c : igd::active_workspace->active_element->v_colors)
	{
		PropertyLabel(ImGui::GetStyleColorName(c.first));
		ImGui::PushItemWidth(item_width);
		if (ImGui::ColorEdit4(getPropertyId("color_" + std::string(ImGui::GetStyleColorName(c.first))), (float*)&c.second, ImGuiColorEditFlags_NoInputs))
		{
			modified = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox(("Inherit##" + std::string(ImGui::GetStyleColorName(c.first))).c_str(), &c.second.inherit))
			modified = true;
	}
	ImGui::EndTable();
}

void Properties::Styles()
{
	ImGui::BeginTable("PropertiesTableStyles", 2, ImGuiTableFlags_SizingFixedFit);
	if (igd::active_workspace->active_element->v_styles.size() > 0)
	{
		PropertySeparator();
		//PropertyLabel("");

		//ImGui::Dummy({ 26, 0 }); ImGui::SameLine();
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Inherit all styles##inherit_styles", &igd::active_workspace->active_element->v_inherit_all_styles))
			modified = true;
		ImGui::TableNextColumn();
	}

	for (auto& c : igd::active_workspace->active_element->v_styles)
	{
		PropertyLabel(ImGuiStyleVar_Strings[c.first]);
		ImGui::PushItemWidth(item_width);
		if (c.second.type == StyleVarType::Float)
		{
			if (ImGui::InputFloat(getPropertyId("style_" + std::string(ImGuiStyleVar_Strings[c.first])), (float*)&c.second.value.Float))
				modified = true;

		}
		else if (c.second.type == StyleVarType::Vec2)
		{
			if (ImGui::InputFloat2(getPropertyId("style_" + std::string(ImGuiStyleVar_Strings[c.first])), (float*)&c.second.value.Vec2))
				modified = true;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox(("Inherit##" + std::string(ImGuiStyleVar_Strings[c.first])).c_str(), &c.second.inherit))
			modified = true;
	}
	ImGui::EndTable();
}

void Properties::OnUIRender() {
	static char* buf = new char[25];
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "Input");
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGui::Begin("Properties");
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	if (ImGui::TreeNodeEx("Element Tree", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginDragDropTarget()) {
			{
				ImGuiElement* source_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
				ImGui::BeginDisabled();
				if (ImGui::TreeNodeEx(source_element->v_id.c_str(), ImGuiTreeNodeFlags_Leaf))
					ImGui::TreePop();
				ImGui::EndDisabled();
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
			{
				std::stringstream ss;
				ImGuiElement* source_element = *(ImGuiElement**)(payload->Data);
				source_element->v_parent = nullptr;
				source_element->PushUndo();
				igd::VecMove(igd::active_workspace->elements, source_element->v_render_index, 0);
			}
			ImGui::EndDragDropTarget();
		}

		for (auto& e : igd::active_workspace->elements)
		{
			if (e->delete_me)
				continue;
			buildTree(e);
		}

		ImGui::TreePop();
	}

	
	if (!igd::active_workspace->active_element)
		igd::active_workspace->active_element = igd::active_workspace->basic_workspace_element;
	is_workspace = igd::active_workspace->active_element == igd::active_workspace->basic_workspace_element;
	if (igd::active_workspace->active_element)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("General"))
		{
			General();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Colors"))
		{
			Colors();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Styles"))
		{
			Styles();
			ImGui::TreePop();
		}
		
		if (ImGui::Button("Delete##property_delete"))
			igd::active_workspace->active_element->Delete();
	}

	
	ImGui::End();
}