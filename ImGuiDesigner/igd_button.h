#pragma once
#include "ImGuiElement.h"
#include <string>
#include <map>
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <map>


namespace igd
{
	class Button : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "button";
		Button() {
			v_icon = igd::textures.images["button"];
			v_tooltip = "Button";
			v_type_id = (int)element_type::button;
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiButtonFlags_None;
			v_property_flags = property_flags::disabled | property_flags::pos;
			v_size = ImVec2(0, 0);
			v_id = ("new button##" + RandomID()).c_str();
			v_label = "new button";
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_Button] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_colors[ImGuiCol_ButtonHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_colors[ImGuiCol_ButtonActive] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_styles[ImGuiStyleVar_FrameBorderSize] = g.Style.FrameBorderSize;
			v_styles[ImGuiStyleVar_ItemInnerSpacing] = g.Style.ItemInnerSpacing;
			v_styles[ImGuiStyleVar_ItemSpacing] = g.Style.ItemSpacing;
			v_styles[ImGuiStyleVar_ButtonTextAlign] = g.Style.ButtonTextAlign;
			v_can_have_children = false;
		}

		virtual ImGuiElement* Clone() override
		{
			Button* new_element = new Button();
			*new_element = *this;
			std::string new_id = this->v_id;
			if (new_id.find("##") != std::string::npos)
				new_id = new_id.substr(0, new_id.find("##") + 2);
			if (igd::active_workspace->GetSingleSelection()->v_can_have_children)
				new_element->v_parent = igd::active_workspace->GetSingleSelection();
			else
				new_element->v_parent = igd::active_workspace->GetSingleSelection()->v_parent;
			new_element->v_id = new_id + RandomID();
			return new_element;
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{

		}

		virtual std::string RenderHead(bool script_only) override
		{
			ImGuiContext& g = *GImGui;
			return "";
		}

		std::string ScriptHead() { return ""; };
		std::string ScriptInternal() { 
			std::stringstream code;
			code << "if (ImGui::Button(\"" << v_id << "\", " << igd::script::GetSizeScript(this) << "))" << std::endl;
			code << "{" << std::endl << "}";
			return code.str();
		};
		
		std::string ScriptFoot() { return ""; };
		virtual std::string RenderInternal(bool script_only) override
		{
			if (script_only)
				return ScriptInternal();

			ImGuiContext& g = *GImGui;
			if (v_id == "")
				return "";
			
			ImGui::Button(v_id.c_str(), this->GetSize());
			
			return ScriptInternal();
		}

		virtual std::string RenderFoot(bool script_only) override
		{
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
			std::cout << "Adding a button" << std::endl;
			igd::Button* b = new igd::Button();
			ImGuiElement* f = (ImGuiElement*)b;
			f->v_parent = parent;
			b->FromJSON(data);
			if (!parent)
				igd::active_workspace->AddNewElement((ImGuiElement*)b, true);
			else
				parent->children.push_back((ImGuiElement*)b);
			return f;
		}
};
}