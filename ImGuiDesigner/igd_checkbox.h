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
	class CheckBox : ImGuiElement
	{
	public:

		static inline std::string json_identifier = "checkbox";
		bool is_checked;
		CheckBox() {
			v_type_id = (int)element_type::checkbox;
			is_checked = false;
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::label | property_flags::disabled;
			v_size = ImVec2(0, 0);
			v_id = ("CheckBox##" + RandomID()).c_str();
			v_label = "";

			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_FrameBg] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			v_colors[ImGuiCol_FrameBgActive] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);
			v_colors[ImGuiCol_FrameBgHovered] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			v_colors[ImGuiCol_TextSelectedBg] = ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg);
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_can_have_children = false;
		}

		virtual ImGuiElement* Clone() override
		{
			CheckBox* new_element = new CheckBox();
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
			code << "static bool " << this->GetIDForVariable() << ";" << std::endl;
			code << "if (ImGui::CheckBox(\"" << v_label << "##" << v_id << "\", &" << this->GetIDForVariable() << "))" << std::endl;
			code << "{" << std::endl << "//checkbox click event" << std::endl << "}";
			return code.str();
		};

		std::string ScriptFoot() { return ""; };
		virtual std::string RenderInternal(bool script_only) override
		{
			if (script_only)
				return ScriptInternal();
			ImGuiContext& g = *GImGui;
			ImGui::Checkbox((v_label + "##" + v_id).c_str(), &is_checked);
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
			std::cout << "Adding a CheckBox" << std::endl;
			igd::CheckBox* b = new igd::CheckBox();
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