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
#include <boost/algorithm/string.hpp>
#include "../json/single_include/nlohmann/json.hpp"
#include <fstream>

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
		if (element == igd::active_workspace->GetSingleSelection() || !element->v_can_have_children || element->delete_me)
			continue;
		if (element->children.size() > 0)
			getChildParents(element);
		if (ImGui::Selectable(element->v_id.c_str()))
		{
			igd::active_workspace->GetSingleSelection()->PushUndo();
			igd::active_workspace->GetSingleSelection()->v_parent = element;
		}
	}
}
void Properties::getAllChildren(ImGuiElement* parent)
{
	for (auto& element : parent->children)
	{
		if (element == igd::active_workspace->GetSingleSelection() || element->delete_me)
			continue;
		if (element->children.size() > 0)
			getAllChildren(element);
		if (ImGui::Selectable(element->v_id.c_str()))
			igd::active_workspace->SetSingleSelection(element);
	}
}
std::vector<std::string> GetIDSplit(std::string id)
{
	std::vector<std::string> sp_id;
	boost::split(sp_id, id, boost::is_any_of("##"));
	return sp_id;
}
std::string GetFlagName(std::string flag)
{
	std::vector<std::string> sp_flag;
	boost::split(sp_flag, flag, boost::is_any_of("_"));
	if (sp_flag.size() > 0)
		return sp_flag[sp_flag.size() - 1];
	else
		return "";
}
void Properties::buildTree(ImGuiElement* current_element)
{
	if (!current_element)
		return;
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

	for (auto& e : igd::active_workspace->selected_elements)
	{
		if (e == current_element)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
			break;
		}
	}
	flags |= ImGuiTreeNodeFlags_DefaultOpen;
	
	std::stringstream ss;
	std::vector<std::string> id = GetIDSplit(current_element->v_id);
	ss << id[0];// GetIDNoPound(current_element->v_id);
	if (current_element->v_can_have_children)
		ss << " (" << std::count_if(current_element->children.begin(), current_element->children.end(), [](ImGuiElement* e) { return !e->delete_me; }) << ")";
	if (id.size()>=1)
		ss << "##" << id[id.size() - 1];


	//if (ImGui::GetDragDropPayload())
	bool IsOpen = ImGui::TreeNodeEx(ss.str().c_str(), flags);
	{
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			igd::active_workspace->SelectElement(current_element);

		if (ImGui::BeginDragDropTarget()) {

			ImGuiElement* source_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
			drop_target = current_element;
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
			{
				std::stringstream ss;
				ImGuiElement* source_element = *(ImGuiElement**)(payload->Data);


				bool parent_changed = false;
				//dragged onto an element that can have children just add it to the children
				int new_index = current_element->v_render_index;

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

				if (current_element->children.size() > 0)
					new_index = 0;
				else if (current_element->v_render_index < source_element->v_render_index || parent_changed)
					new_index += 1;


				if (!parent_changed)
				{
					//if (source_element->v_parent)
						igd::VecMove(source_element->v_parent->children, source_element->v_render_index, new_index);
//					else
	//					igd::VecMove(igd::active_workspace->elements, source_element->v_render_index, new_index);
				}
				else
				{
					source_element->v_render_index = new_index;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("_TREENODE", &current_element, sizeof(current_element));
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
			if (current_element == drop_target && ImGui::GetDragDropPayload() && current_element->v_can_have_children)
			{
				ImGuiElement* source_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
				ImGui::BeginDisabled();
				if (ImGui::TreeNodeEx(source_element->v_id.c_str(), ImGuiTreeNodeFlags_Leaf))
					ImGui::TreePop();
				ImGui::EndDisabled();
			}
			ImGui::TreePop();
		}


		if (current_element == drop_target && ImGui::GetDragDropPayload() && !current_element->v_can_have_children)
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
	return ("##property_" + name + "_" + igd::active_workspace->GetSingleSelection()->v_id).c_str();
}
void Properties::General()
{
	//static Walnut::Image img = Walnut::Image("D:\\3xp.png");
	//ImGui::Image(img.GetDescriptorSet(), { (float)img.GetWidth(), (float)img.GetHeight() });

	ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_SizingFixedFit);
	if (!(igd::active_workspace->GetSingleSelection()->v_property_flags & property_flags::no_id))
	{
		PropertyLabel("ID:");
		ImGui::PushItemWidth(item_width);
		ImGui::InputText("##property_id", &igd::active_workspace->GetSingleSelection()->v_id);
	}
	if (igd::active_workspace->GetSingleSelection()->v_property_flags & property_flags::label)
	{
		PropertyLabel("Label:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::InputText(getPropertyId("label"), &igd::active_workspace->GetSingleSelection()->v_label))
		{
			igd::active_workspace->GetSingleSelection()->PushUndo();
		}
	}


	PropertyLabel("Font:");
	ImGui::PushItemWidth(item_width);
	std::filesystem::path font_path = igd::active_workspace->GetSingleSelection()->v_font.path;
	static bool do_update = false;
	if (ImGui::BeginCombo(getPropertyId("font"), font_path.stem().string() == "" ? "Inherit" : font_path.stem().string().c_str()))
	{
		if (do_update)
		{
			igd::font_manager->UpdateFonts();
			do_update = false;
		}

		if (ImGui::Selectable("Inherit"))
		{
			igd::active_workspace->GetSingleSelection()->v_font.name = "";
			igd::active_workspace->GetSingleSelection()->v_font.font = nullptr;
			igd::active_workspace->GetSingleSelection()->PushUndo();
		}
		//std::vector<std::pair<std::string, Font>> A;
		//for (auto& f : igd::font_manager->AvailableFonts)
		//{
		//	A.push_back(f);
		//}
		//std::sort(A.begin(), A.end());
		std::sort(igd::font_manager->AvailableFonts.begin(), igd::font_manager->AvailableFonts.end());
		for (auto& [name, f] : igd::font_manager->AvailableFonts)
		{
			if (!f.valid)
				ImGui::Text("%s (invalid)", name.c_str());
			else
			{
				bool clicked = ImGui::Selectable(name.c_str());
			/*	if (f.hasSample())
				{
					ImGui::SameLine();
					f.draw_sample();
				}*/
				//else if (ImGui::IsItemVisible())
				//	igd::font_manager->LoadFont(f._path, 20, nullptr);
				
				if (clicked)
				{
					igd::font_manager->LoadFont(f._path, igd::active_workspace->GetSingleSelection()->v_font.size, igd::active_workspace->GetSingleSelection());
				}
			}
		}
		ImGui::EndCombo();
	}
	else
		do_update = true;

	PropertyLabel("Font Size:");
	ImGui::PushItemWidth(item_width);
	if (ImGui::InputInt(getPropertyId("font_size"), &igd::active_workspace->GetSingleSelection()->v_font.size, 0, 0) && igd::active_workspace->GetSingleSelection()->v_font.size > 0 && igd::active_workspace->GetSingleSelection()->v_font.font)
		igd::font_manager->LoadFont(igd::active_workspace->GetSingleSelection()->v_font.path, igd::active_workspace->GetSingleSelection()->v_font.size, igd::active_workspace->GetSingleSelection());

	if (!(igd::active_workspace->GetSingleSelection()->v_property_flags & property_flags::no_resize))
	{
		PropertyLabel("Size:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::InputFloat2(getPropertyId("size"), (float*)&igd::active_workspace->GetSingleSelection()->v_size.value))
		{
			igd::active_workspace->GetSingleSelection()->PushUndo();
		}
		ImGui::SameLine();
		bool checked = igd::active_workspace->GetSingleSelection()->v_size.type == Vec2Type::Relative;
		if (ImGui::Checkbox("%##size_pct", &checked))
		{
			if (!checked)
				igd::active_workspace->GetSingleSelection()->v_size.type = Vec2Type::Absolute;
			else
				igd::active_workspace->GetSingleSelection()->v_size.type = Vec2Type::Relative;
		}
	}

	if (igd::active_workspace->GetSingleSelection()->v_property_flags & property_flags::pos)
	{
		PropertyLabel("Position:");
		ImGui::PushItemWidth(item_width);
		if (ImGui::InputFloat2(getPropertyId("pos"), (float*)&igd::active_workspace->GetSingleSelection()->v_pos.value))
		{
			igd::active_workspace->GetSingleSelection()->PushUndo();
		}
		ImGui::SameLine();
		bool checked = igd::active_workspace->GetSingleSelection()->v_pos.type == Vec2Type::Relative;
		if (ImGui::Checkbox("%##pos_pct", &checked))
		{
			if (!checked)
				igd::active_workspace->GetSingleSelection()->v_pos.type = Vec2Type::Absolute;
			else
				igd::active_workspace->GetSingleSelection()->v_pos.type = Vec2Type::Relative;
		}
		/*if (ImGui::BeginTabBar(getPropertyId("pos_type"), igd::active_workspace->GetSingleSelection()->v_pos.type == Vec2Type::Absolute ? "Absolute" : "Relative (%)"))
		{
			if (ImGui::Selectable("Absolute"))
				igd::active_workspace->GetSingleSelection()->v_pos.type = Vec2Type::Absolute;
			if (ImGui::Selectable("Relative (%)"))
				igd::active_workspace->GetSingleSelection()->v_pos.type = Vec2Type::Relative;
			ImGui::EndTabBar();
		}*/
	}

	if (!is_workspace)
	{
		PropertyLabel("Sameline:");
		ImGui::Checkbox("##property_sameline", &igd::active_workspace->GetSingleSelection()->v_sameline);
	}

	
	if (modified && !ImGui::IsPopupOpen("picker", ImGuiPopupFlags_AnyPopup))
	{
		modified = false;
		igd::active_workspace->GetSingleSelection()->PushUndo();
	}
	if (igd::active_workspace->GetSingleSelection()->v_property_flags & property_flags::disabled && !is_workspace)
	{
		PropertyLabel("Disabled:");
		if (ImGui::Checkbox("##property_disabled", &igd::active_workspace->GetSingleSelection()->v_disabled))
		{
			igd::active_workspace->GetSingleSelection()->PushUndo();
		}
	}

//	if (igd::active_workspace->GetSingleSelection()->v_can_have_children && igd::active_workspace->GetSingleSelection()->children.size() > 0)
//	{
//	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("A widget will be saved comprised of all the children of this element.\nThe name will be the id of this element.");
		ImGui::EndTooltip();
	}
	
	ImGui::EndTable();
}
bool Properties::ColorSelector(ImVec4 color, std::string title)
{
	ImGui::TableNextColumn();
	bool rval = false;
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
	if (ImGui::Button(("##color_palette_color_" +  title).c_str(), {20,20}))
		rval = true;
	ImGui::TableNextColumn();
	ImGui::Text(title.c_str());
	ImGui::PopStyleColor(3);
	return rval;
}
void Properties::Colors()
{
	ImGui::BeginTable("PropertiesTableColors", 2, ImGuiTableFlags_SizingFixedFit);
	if (igd::active_workspace->GetSingleSelection()->v_colors.size() > 0)
	{
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Inherit all Colors##inherit_colors", &igd::active_workspace->GetSingleSelection()->v_inherit_all_colors))
			modified = true;
		ImGui::TableNextColumn();
	}
	for (auto& c : igd::active_workspace->GetSingleSelection()->v_colors)
	{
		PropertyLabel(ImGui::GetStyleColorName(c.first));
		ImGui::PushItemWidth(item_width);
		if (ImGui::ColorEdit4(getPropertyId("color_" + std::string(ImGui::GetStyleColorName(c.first))), (float*)&c.second, ImGuiColorEditFlags_NoInputs))
		{
			modified = true;
		}
		if (ForcePicker)
		{
			ImGui::OpenPopup("picker1");
			std::cout << "Open picker popup" << std::endl;
			ForcePicker = false;
		}


		if (ImGui::IsItemHovered() && ImGui::IsItemClicked())
		{
			std::cout << "changed active color" << std::endl;
			active_color = (ImColor*)&c.second;
			if (igd::active_workspace->GetSingleSelection()->v_inherit_all_colors || c.second.inherit)
				igd::dialogs->Confirmation("Warning", "This element is inheriting its color from its parent, would you like to override it?", "", [&c, this](bool override) {
				if (override)
				{
					this->ForcePicker = true;
					igd::active_workspace->GetSingleSelection()->v_inherit_all_colors = false;
					c.second.inherit = false;
				}
			});
			
			
		}
		ImGui::SameLine();
		if (ImGui::Checkbox(("Inherit##" + std::string(ImGui::GetStyleColorName(c.first))).c_str(), &c.second.inherit))
			modified = true;
	}
	float square_sz = ImGui::GetFrameHeight();
	if (ImGui::BeginPopup("picker1"))
	{
		ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
		ImGuiColorEditFlags picker_flags = (ImGuiColorEditFlags_NoInputs & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
		ImGui::SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
		ImGui::ColorPicker4("##picker1", (float*)active_color, picker_flags, &GImGui->ColorPickerRef.x);
		ImGui::EndPopup();
	}
	ImGui::EndTable();
}
void Properties::Styles()
{
	ImGui::BeginTable("PropertiesTableStyles", 2, ImGuiTableFlags_SizingFixedFit);
	if (igd::active_workspace->GetSingleSelection()->v_styles.size() > 0)
	{
		PropertySeparator();
		ImGui::TableNextColumn();
		if (ImGui::Checkbox("Inherit all styles##inherit_styles", &igd::active_workspace->GetSingleSelection()->v_inherit_all_styles))
			modified = true;
		ImGui::TableNextColumn();
	}

	for (auto& c : igd::active_workspace->GetSingleSelection()->v_styles)
	{
		PropertyLabel(GetFlagName(ImGuiStyleVar_Strings[c.first]).c_str());
		ImGui::PushItemWidth(item_width);
		if (c.second.type == StyleVarType::Float)
		{
			if (ImGui::InputFloat(getPropertyId("style_" + std::string(ImGuiStyleVar_Strings[c.first])), (float*)&c.second.value.Float))
			{
				modified = true;
				c.second.inherit = false;
			}

		}
		else if (c.second.type == StyleVarType::Vec2)
		{
			if (ImGui::InputFloat2(getPropertyId("style_" + std::string(ImGuiStyleVar_Strings[c.first])), (float*)&c.second.value.Vec2))
			{
				modified = true;
				c.second.inherit = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Checkbox(("Inherit##" + std::string(ImGuiStyleVar_Strings[c.first])).c_str(), &c.second.inherit))
			modified = true;
	}
	ImGui::EndTable();
}
void Properties::Flags()
{
	ImGui::BeginTable("PropertiesTableFlags", 2, ImGuiTableFlags_SizingFixedFit);
	for (auto& [flag, str] : igd::active_workspace->GetSingleSelection()->v_custom_flags)
	{
		PropertyLabel(GetFlagName(str).c_str());
		ImGui::PushItemWidth(item_width);
		ImGui::CheckboxFlags(("##" + str).c_str(), &igd::active_workspace->GetSingleSelection()->v_flags, flag);
	}
	ImGui::EndTable();
}
void Properties::Tree()
{
	std::string title = STS() << "Element Tree (" << std::count_if(igd::active_workspace->basic_workspace_element->children.begin(), igd::active_workspace->basic_workspace_element->children.end(), [](ImGuiElement* e) { return !e->delete_me; }) << ")";
	bool tn = (ImGui::TreeNodeEx(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen));
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Drag and drop here in the element tree to change parents and re-sort your elements.");
		ImGui::EndTooltip();
	}
	if (tn)
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
				ImGuiElement* source_element = *(ImGuiElement**)(payload->Data);
				source_element->v_parent = igd::active_workspace->basic_workspace_element;
				igd::VecMove(source_element->v_parent->children, source_element->v_render_index, 0);
				//source_element->v_render_index = 0;
				source_element->PushUndo();
			}
			ImGui::EndDragDropTarget();
		}

		for (auto& e : igd::active_workspace->basic_workspace_element->children)
		{
			if (e->delete_me)
				continue;
			buildTree(e);
		}

		static bool is_last_hovered = false;
		ImGuiElement* source_element = nullptr;
		if (ImGui::GetDragDropPayload())
			source_element = *(ImGuiElement**)(ImGui::GetDragDropPayload()->Data);
		std::string title = "##some_invis_leaf";
		if (source_element && is_last_hovered)
		{
			title = source_element->v_id + "##some_invis_leaf";
		}

		ImGui::BeginDisabled();
		if (ImGui::TreeNodeEx(title.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			if (ImGui::BeginDragDropTarget())
			{
				is_last_hovered = true;
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TREENODE"))
				{
					ImGuiElement* source_element = *(ImGuiElement**)(payload->Data);
					source_element->v_parent = igd::active_workspace->basic_workspace_element;
					std::cout << "Moved index: " << source_element->v_render_index << " to " << igd::active_workspace->basic_workspace_element->children.size() << std::endl;
					igd::VecMove(source_element->v_parent->children, source_element->v_render_index, igd::active_workspace->basic_workspace_element->children.size() - 1);
					source_element->v_render_index = (int)igd::active_workspace->basic_workspace_element->children.size() - 1;
					source_element->PushUndo();
					//	igd::VecMove(source_element->v_parent->children, source_element->v_render_index, (int)igd::active_workspace->basic_workspace_element->children.size()-1);
				}
				ImGui::EndDragDropTarget();
			}
			else
				is_last_hovered = false;
			ImGui::TreePop();
		}
		ImGui::EndDisabled();

		ImGui::TreePop();


	}
}
void Properties::OnUIRender() {
	static char* buf = new char[25];
	memset(buf, 0, 25);
	strcpy_s(buf, 25, "Input");
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	if (!igd::active_workspace->GetSingleSelection())
		igd::active_workspace->SetSingleSelection(igd::active_workspace->basic_workspace_element);
	is_workspace = igd::active_workspace->GetSingleSelection() == igd::active_workspace->basic_workspace_element;
	

	igd::push_designer_theme();
	ImGui::Begin("Properties");
	
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	if (ImGui::BeginTabBar("Properties_Tabs"))
	{
		if (ImGui::BeginTabItem("Element Tree"))
		{
			Tree();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Properties"))
		{
			if (igd::active_workspace->GetSingleSelection())
			{
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNode("General"))
				{
					General();
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Element Specific"))
				{
					ImGui::BeginTable("PropertiesElementSpecific", 2, ImGuiTableFlags_SizingFixedFit);

					igd::active_workspace->GetSingleSelection()->RenderPropertiesInternal();
					ImGui::EndTable();
					ImGui::TreePop();
				}
				if (igd::active_workspace->GetSingleSelection()->v_colors.size() > 0)
				{
					if (ImGui::TreeNode("Colors"))
					{
						Colors();
						ImGui::TreePop();
					}
				}
				if (igd::active_workspace->GetSingleSelection()->v_styles.size() > 0)
				{
					if (ImGui::TreeNode("Styles"))
					{
						Styles();
						ImGui::TreePop();
					}
				}
				if (igd::active_workspace->GetSingleSelection()->v_custom_flags.size() > 0)
				{
					if (ImGui::TreeNode("Flags"))
					{
						Flags();
						ImGui::TreePop();
					}
				}

			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	float button_height = 45;
	ImGui::Dummy({ 0, ImGui::GetContentRegionAvail().y-button_height- GImGui->Style.FramePadding.y });
		//ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetContentRegionMax().y - GImGui->Style.FramePadding.y - button_height });
		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x / 2 - GImGui->Style.FramePadding.x;
		if (ImGui::Button("Save as Widget##Json_Save", { width, button_height }))
		{
			igd::dialogs->SaveWidget(igd::active_workspace->GetSingleSelection()->v_id + "\nSave as widget" , { "filename", "short description" }, "", { "Save", "Cancel" }, [](bool do_save, std::vector<std::string> rval, std::string icon_name)
				{
					if (do_save)
						igd::active_workspace->GetSingleSelection()->SaveAsWidget(rval[0], rval[1], icon_name);
				});
		}

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("When you save as a widget it will save all of its children as well");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete##property_delete", { width, button_height }))
			igd::active_workspace->GetSingleSelection()->Delete();
	

		ImGui::End();

	
	
	//some trickery to expand the color picker popup
	if (active_color)
	{
		for (auto& f : g.OpenPopupStack)
		{
			if (!f.Window)
				continue;
			std::string name = f.Window->Name;
			if (name.substr(0, 7) == "##Popup" && f.Window->ParentWindow && std::string(f.Window->ParentWindow->Name) == "Properties")
			{
				ImGui::Begin(name.c_str());
				if (ImGui::Button("Open Palette"))
					ImGui::OpenPopup("color_palette");

				if (!this->color_palette.has_filename())
				{
					for (auto& p : std::filesystem::directory_iterator("palettes"))
					{
						if (p.path().extension() == ".igp")
						{
							this->LoadPalette(p.path());
							break;
						}
					}
				}
				if (ImGui::BeginPopup("color_palette", ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
				{
					if (!std::filesystem::exists("palettes"))
						std::filesystem::create_directory("palettes");
					if (ImGui::BeginCombo("Palettes", this->color_palette.stem().string().c_str()))
					{
						std::filesystem::path delete_path;
						//iterate over all files with extension igp in the palettes folder
						for (auto& p : std::filesystem::directory_iterator("palettes"))
						{
							if (p.path().extension() == ".igp")
							{
								if (ImGui::Selectable(p.path().stem().string().c_str(), (p.path() == this->color_palette)))
									this->LoadPalette(p.path());
								if (ImGui::BeginPopupContextItem(("##context_menu_palette_" + p.path().stem().string()).c_str()))
								{
									if (ImGui::MenuItem("Delete"))
										delete_path = p.path();
									ImGui::EndPopup();
								}
							}
						}
						if (delete_path.has_filename())
						{
							igd::dialogs->Confirmation("Delete Palette", "Delete Palette " + delete_path.stem().string() + "?", "", [delete_path, this](bool rval) {
								if (rval)
								{
									std::filesystem::remove(delete_path);
									if (delete_path == this->color_palette)
									{
										this->color_palette.clear();
										this->color_palette_colors.clear();
									}
								}
							});
						}
						if (ImGui::Selectable("New Palette"))
						{
							igd::dialogs->InputText("New Palette", "Name your new palette", "", { "Save", "Cancel" }, [this](bool save, std::string name) {
								if (save)
								{
									std::cout << "Saving palette " << name << std::endl;
									std::filesystem::path path = "palettes/" + name + ".igp";
									if (std::filesystem::exists(path))
										igd::dialogs->Confirmation("Overwrite File", "Are you sure you wish to overwrite\n" + path.string(), "", [path, this](bool result) {
											if (result)
											{
												this->color_palette_colors.clear();
												this->color_palette = path;
												this->SavePalette();
											}
										});
									else
									{
										this->color_palette_colors.clear();
										this->color_palette = path;
										std::cout << "generate new file" << std::endl;
										this->SavePalette();
									}
								}
							});
						}
						ImGui::EndCombo();
					}

					int delete_index = -1;
					if (ImGui::BeginTable("##color_palette_table", 2))
					{
						for (int index = 0; auto& c : this->color_palette_colors)
						{
							if (ColorSelector(c.color, c.name.c_str()))
								*active_color = c.color;
							if (ImGui::BeginPopupContextItem(("##context_menu_palette_" + c.name).c_str()))
							{
								if (ImGui::MenuItem("Delete"))
									delete_index = index;
								ImGui::EndPopup();
							}
							index++;
						}
						if (delete_index != -1)
						{
							this->color_palette_colors.erase(this->color_palette_colors.begin() + delete_index);
							this->SavePalette();
						}
						if (this->color_palette.has_filename() && ColorSelector(*active_color, "Add to Palette"))
						{
							igd::dialogs->InputText("Color Name", "Name for color", "", { "Save", "Cancel" }, [this](bool save, std::string name) {
								if (save)
								{
									this->color_palette_colors.push_back({ name, *this->active_color });
									this->SavePalette();
								}
								});
						}
	
						ImGui::EndTable();
					}
					igd::dialogs->OnUIRender();
					if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
					{
						igd::UnPressKey(ImGuiKey_Enter);
						igd::UnPressKey(ImGuiKey_KeyPadEnter);
						igd::UnPressKey(ImGuiKey_Escape);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::End();
				
			}

		}
	}


	igd::pop_designer_theme();
}
void Properties::LoadPalette(std::filesystem::path palette_path)
{
	this->color_palette_colors.clear();
	this->color_palette = palette_path;
	std::ifstream i(palette_path);
	try
	{
		nlohmann::json json_file = nlohmann::json::parse(i);
		int elements = 0;
		for (auto& color : json_file)
		{
			this->color_palette_colors.push_back({ color["name"], ImColor((float)color["color"]["x"],(float)color["color"]["y"],(float)color["color"]["z"],(float)color["color"]["w"]) });
		}
	}
	catch (nlohmann::json::exception& ex)
	{
		igd::dialogs->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
		std::cerr << "parse error at byte " << ex.what() << std::endl;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		igd::dialogs->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
		std::cerr << "parse error at byte " << ex.byte << std::endl << ex.what() << std::endl;
	}
}
void Properties::SavePalette()
{

	std::ofstream file;
	file.open(this->color_palette);
	nlohmann::json obj = nlohmann::json::array();
	for (auto& c : color_palette_colors)
	{
		nlohmann::json color;
		color["name"] = c.name;
		color["color"] = { { "x", c.color.x },{ "y", c.color.y }, { "z", c.color.z }, { "w", c.color.w } };
		obj.push_back(color);
	}
	file << obj.dump() << std::endl;
	std::cout << obj.dump() << std::endl;
	file.close();
	
}