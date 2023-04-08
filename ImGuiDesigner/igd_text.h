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
	class Text : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "text";
		Text() {
			v_type_id = (int)element_type::text;
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiButtonFlags_None;
			v_property_flags = property_flags::disabled | property_flags::pos | property_flags::label | property_flags::no_id | property_flags::no_resize;
			v_size = ImVec2(0, 0);
			v_id = "Text Element";// ("new text##" + RandomID()).c_str();
			v_label = "Some text value";
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_can_have_children = false;
		}
		virtual ImGuiElement* Clone() override
		{
			Text* new_element = new Text();
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

		std::string ScriptHead() {
			return "";
		};
		std::string ScriptInternal() {
			std::stringstream code;
			code << "ImGui::Text(\"" << v_label << "\");";
			return code.str();
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
			if (v_id == "")
				return "";
			ImGui::Text(v_label.c_str());
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
			std::cout << "Adding text" << std::endl;
			igd::Text* b = new igd::Text();
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