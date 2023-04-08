#pragma once
#include "ImGuiElement.h"
#include <string>
#include <vector>
#include "Workspace.h"

namespace igd
{
	class TabItem : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "TabItem";
		TabItem() {
			v_type_id = (int)element_type::tabitem;
			v_can_contain_own_type = false;
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::disabled | property_flags::border;
			v_size = ImVec2(0, 0);
			v_id = ("TabItem##" + RandomID()).c_str();
			v_label = "";
			v_border = true;
			v_can_have_children = true;
			v_requires_open = true;
			v_is_open = false;
			v_parent_required_id = (int)element_type::tabbar;
			v_auto_select = false;
			//available child window flags
			v_custom_flags[ImGuiTabItemFlags_UnsavedDocument] = "ImGuiTabItemFlags_UnsavedDocument";
			v_custom_flags[ImGuiTabItemFlags_SetSelected] = "ImGuiTabItemFlags_SetSelected";
			v_custom_flags[ImGuiTabItemFlags_NoCloseWithMiddleMouseButton] = "ImGuiTabItemFlags_NoCloseWithMiddleMouseButton";
			v_custom_flags[ImGuiTabItemFlags_NoPushId] = "ImGuiTabItemFlags_NoPushId";
			v_custom_flags[ImGuiTabItemFlags_NoReorder] = "ImGuiTabItemFlags_NoReorder";
			v_custom_flags[ImGuiTabItemFlags_Leading] = "ImGuiTabItemFlags_Leading";
			v_custom_flags[ImGuiTabItemFlags_Trailing] = "ImGuiTabItemFlags_Trailing";

			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_Border] = ImGui::GetStyleColorVec4(ImGuiCol_Border);
			v_colors[ImGuiCol_BorderShadow] = ImGui::GetStyleColorVec4(ImGuiCol_BorderShadow);
			v_colors[ImGuiCol_FrameBg] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			v_colors[ImGuiCol_FrameBgActive] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);
			v_colors[ImGuiCol_FrameBgHovered] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			v_colors[ImGuiCol_ScrollbarBg] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarBg);
			v_colors[ImGuiCol_ScrollbarGrab] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrab);
			v_colors[ImGuiCol_ScrollbarGrabActive] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);
			v_colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabHovered);
			v_colors[ImGuiCol_Separator] = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
			v_colors[ImGuiCol_SeparatorActive] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive);
			v_colors[ImGuiCol_SeparatorHovered] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered);
			v_colors[ImGuiCol_TextSelectedBg] = ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg);
			v_colors[ImGuiCol_Tab] = ImGui::GetStyleColorVec4(ImGuiCol_Tab);
			v_colors[ImGuiCol_TabActive] = ImGui::GetStyleColorVec4(ImGuiCol_TabActive);
			v_colors[ImGuiCol_TabHovered] = ImGui::GetStyleColorVec4(ImGuiCol_TabHovered);

			v_styles[ImGuiStyleVar_DisabledAlpha] = g.Style.DisabledAlpha;
			v_styles[ImGuiStyleVar_FrameBorderSize] = g.Style.FrameBorderSize;
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_styles[ImGuiStyleVar_GrabMinSize] = g.Style.GrabMinSize;
			v_styles[ImGuiStyleVar_GrabRounding] = g.Style.GrabRounding;
			v_styles[ImGuiStyleVar_ItemInnerSpacing] = g.Style.ItemInnerSpacing;
			v_styles[ImGuiStyleVar_ItemSpacing] = g.Style.ItemSpacing;
			v_styles[ImGuiStyleVar_ScrollbarRounding] = g.Style.ScrollbarRounding;
			v_styles[ImGuiStyleVar_ScrollbarSize] = g.Style.ScrollbarSize;
			v_styles[ImGuiStyleVar_SelectableTextAlign] = g.Style.SelectableTextAlign;
			v_styles[ImGuiStyleVar_TabRounding] = g.Style.TabRounding;
		}
		virtual ImGuiElement* Clone() override
		{
			TabItem* new_element = new TabItem();
			*new_element = *this;
			std::string new_id = this->v_id;
			if (new_id.find("##") != std::string::npos)
				new_id = new_id.substr(0, new_id.find("##") + 2);

			if (igd::active_workspace->GetSingleSelection()->v_can_have_children)
				new_element->v_parent = igd::active_workspace->GetSingleSelection();
			else
				new_element->v_parent = igd::active_workspace->GetSingleSelection()->v_parent;

			new_element->v_id = new_id + RandomID();
			new_element->children.clear();
			for (auto& child : this->children)
			{
				ImGuiElement* n = child->Clone();
				n->v_parent = new_element;
				new_element->children.push_back(n);
			}
			return new_element;
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{

		}

		std::string ScriptHead() {
			std::stringstream code;
			code << "if (ImGui::BeginTabItem(\"" << v_id << "\", " << igd::script::BuildFlagString(this) << "))";
			return code.str();
		};
		std::string ScriptInternal() {
			return "ImGui::EndTabItem();";
		};

		std::string ScriptFoot() { return ""; };
		
		virtual std::string RenderHead(bool script_only) override
		{
			
			if (v_id == "")
				return "";
			
			if (script_only)
				return ScriptHead();
			
			v_is_open = ImGui::BeginTabItem(v_id.c_str(), nullptr, v_flags);
			return ScriptHead();
		}
		virtual std::string RenderInternal(bool script_only) override
		{
			//iterate all children handled by imguielement cpp
			return ScriptInternal();
		}
		virtual std::string RenderFoot(bool script_only) override
		{
			if (v_id == "")
				return "";
			if (v_is_open && !script_only)
			{

				////a little bug fix for imgui hover when in tabs
				//ImVec2 cp = ImGui::GetCursorPos();
				//ImGui::Dummy({ 50,50 });
				//ImGui::SetCursorPos(cp);
				ImGui::EndTabItem();
			}
			return "";
		}
		virtual void FromJSON(nlohmann::json data) override
		{
			StylesColorsFromJson(data);
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			GenerateStylesColorsJson(j, json_identifier);
			return j;
		}

		static ImGuiElement* load(ImGuiElement* parent, nlohmann::json data)
		{
			ImGuiElement* new_parent = nullptr;
			std::cout << "TabItem found" << std::endl;
			igd::TabItem* b = new igd::TabItem();
			b->FromJSON(data);
			new_parent = (ImGuiElement*)b;
			if (parent)
				parent->children.push_back((ImGuiElement*)b);
			else
				igd::active_workspace->AddNewElement((ImGuiElement*)b, true);
			new_parent->v_parent = parent;
			return new_parent;
		}
	};
}