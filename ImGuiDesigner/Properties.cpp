#include "Properties.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <Windows.h>
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

std::filesystem::path getWindowsFontsDirectory()
{
	//get windows font directory
	char windir[MAX_PATH];
	GetWindowsDirectoryA(windir, MAX_PATH);
	std::string fontdir = windir;
	fontdir += "\\Fonts\\";
	return fontdir;
}
void Properties::OnUpdate(float f)
{
	if (load_font)
	{
		if (!LoadedFonts[this->font.string()])
			LoadedFonts[this->font.string()] = ImGui::GetIO().Fonts->AddFontFromFileTTF(font.string().c_str(), 20.0f);
		active_element->v_font = font.string();
		active_element->PushUndo();
		active_element->v_font_ptr = LoadedFonts[this->font.string()];
		load_font = false;
		igd::notifications->GenericNotification("Loaded", "Loaded font", "", "Ok", []() {});
		ImGui::GetIO().Fonts->Build();
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
		
		PropertyLabel("Font:");
		ImGui::PushItemWidth(item_width);
		std::filesystem::path font_path = active_element->v_font;
		if (ImGui::BeginCombo("##Font", font_path.stem().string() == "" ? "Inherit" : font_path.stem().string().c_str()))
		{
			//create directory if doesn't exist
			std::filesystem::path fonts_dir = "fonts";
			if (!std::filesystem::exists(fonts_dir))
				std::filesystem::create_directory(fonts_dir);
			if (ImGui::Selectable("Inherit"))
			{
				active_element->v_font = "";
				active_element->PushUndo();
			}
			for (auto& p : std::filesystem::directory_iterator(getWindowsFontsDirectory()))
			{
				if (p.path().extension() == ".ttf" && ImGui::Selectable(p.path().stem().string().c_str()))
				{
					load_font = true;
					font = p.path();
				}
			}
			for (auto& p : std::filesystem::directory_iterator(fonts_dir))
			{
				if (p.path().extension() == ".ttf" && ImGui::Selectable(p.path().stem().string().c_str()))
				{
					load_font = true;
					font = p.path();
				}
			}
			ImGui::EndCombo();
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

		if (active_element->v_property_flags & property_flags::pos && !is_workspace)
		{
			PropertyLabel("Position:");
			ImGui::PushItemWidth(item_width);
			if (ImGui::InputFloat2("##property_pos", (float*)&active_element->v_pos))
			{
				active_element->PushUndo();
			}
		}

		if (active_element->v_colors.size() > 0)
		{
			PropertySeparator();
			//PropertyLabel("");
			//ImGui::Dummy({26, 0}); ImGui::SameLine();
			ImGui::TableNextColumn();
			if (ImGui::Checkbox("Inherit all Colors##inherit_colors", &active_element->v_inherit_all_colors))
				modified = true;
			ImGui::TableNextColumn();
		}
		for (auto& c : active_element->v_colors)
		{
			PropertyLabel(ImGui::GetStyleColorName(c.first));
			ImGui::PushItemWidth(item_width);
			if (ImGui::ColorEdit4(("##property_color_" + std::string(ImGui::GetStyleColorName(c.first))).c_str(), (float*)&c.second, ImGuiColorEditFlags_NoInputs))
			{
				modified = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox(("Inherit##" + std::string(ImGui::GetStyleColorName(c.first))).c_str(), &c.second.inherit))
				modified = true;
		}
		
		if (active_element->v_styles.size() > 0)
		{
			PropertySeparator();
			//PropertyLabel("");

			//ImGui::Dummy({ 26, 0 }); ImGui::SameLine();
			ImGui::TableNextColumn();
			if (ImGui::Checkbox("Inherit all styles##inherit_styles", &active_element->v_inherit_all_styles))
				modified = true;
			ImGui::TableNextColumn();
		}
		
		for (auto& c : active_element->v_styles)
		{
			PropertyLabel(ImGuiStyleVar_Strings[c.first]);
			ImGui::PushItemWidth(item_width);
			if (c.second.type == StyleVarType::Float)
			{
				if (ImGui::InputFloat(("##property_style_" + std::string(ImGuiStyleVar_Strings[c.first])).c_str(), (float*)&c.second.value.Float))
					modified = true;
				
			}
			else if (c.second.type == StyleVarType::Vec2)
			{
				if (ImGui::InputFloat2(("##property_style_" + std::string(ImGuiStyleVar_Strings[c.first])).c_str(), (float*)&c.second.value.Vec2))
					modified = true;
			}
			ImGui::SameLine();
			if (ImGui::Checkbox(("Inherit##" + std::string(ImGuiStyleVar_Strings[c.first])).c_str(), &c.second.inherit))
				modified = true;
		}
		if (modified && !ImGui::IsPopupOpen("picker", ImGuiPopupFlags_AnyPopup))
		{
			modified = false;
			active_element->PushUndo();
		}
		PropertySeparator();
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