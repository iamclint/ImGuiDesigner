#pragma once
#include "ImGuiElement.h"
#include <string>
#include <map>
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <map>


namespace igd
{
	class Separator : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "separator";

		Separator() {
			v_type_id = (int)element_type::separator;
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiSeparatorFlags_None;
			v_property_flags = property_flags::pos | property_flags::no_id | property_flags::no_resize;
			v_size = ImVec2(0, 0);
			v_id = ("Separator##" + RandomID()).c_str();
			v_label = "";
			v_colors[ImGuiCol_Separator] = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
			v_colors[ImGuiCol_SeparatorHovered] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered);
			v_colors[ImGuiCol_SeparatorActive] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive);
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_can_have_children = false;
		}
		virtual ImGuiElement* Clone() override
		{
			Separator* new_element = new Separator();
			*new_element = *this;
			std::string new_id = this->v_id;
			if (new_id.find("##") != std::string::npos)
				new_id = new_id.substr(0, new_id.find("##") + 2);
			if (active_workspace->active_element->v_can_have_children)
				new_element->v_parent = active_workspace->active_element;
			else
				new_element->v_parent = active_workspace->active_element->v_parent;
			new_element->v_id = new_id + RandomID();
			return new_element;
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{

		}

		std::string ScriptHead() {
			return "";
		};
		std::string ScriptInternal() {
			return "ImGui::Separator();";
		};

		std::string ScriptFoot() { return ""; };

		virtual std::string RenderHead(bool script_only) override
		{
			ImGuiContext& g = *GImGui;
			return "";
		}

		virtual std::string RenderInternal(bool script_only) override
		{
			if (script_only)
				return ScriptInternal();
			ImGui::Separator();
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
			std::cout << "Adding a Separator" << std::endl;
			igd::Separator* b = new igd::Separator();
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