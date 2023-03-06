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
		static inline std::unordered_map<Text*, std::vector<Text>> undo_stack;
		static inline std::unordered_map<Text*, std::vector<Text>> redo_stack;
		static inline std::string json_identifier = "text";
		Text() {
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiButtonFlags_None;
			v_property_flags = property_flags::disabled | property_flags::pos | property_flags::label | property_flags::no_id | property_flags::no_resize;
			v_size = ImVec2(0, 0);
			v_id = "Text Element";// ("new text##" + RandomID()).c_str();
			v_label = "Some text value";
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_can_have_children = false;
			PushUndo();
		}

		virtual void UndoLocal() override
		{
			if (undo_stack[this].size() > 1)
			{
				redo_stack[this].push_back(*this);
				if (undo_stack[this].size() > 1)
					undo_stack[this].pop_back();

				*this = undo_stack[this].back();
			}
		}
		virtual void RedoLocal() override
		{
			if (redo_stack[this].size() > 0)
			{
				*this = redo_stack[this].back();
				PushUndo();
				redo_stack[this].pop_back();
			}
		}

		virtual void PushUndoLocal() override
		{
			//keep an undo stack locally for this type
			undo_stack[this].push_back(*this);
		}

		virtual ImGuiElement* Clone() override
		{
			Text* new_element = new Text();
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

		virtual std::string RenderHead() override
		{
			ImGuiContext& g = *GImGui;
			return "";
		}

		virtual std::string RenderInternal() override
		{
			ImGuiContext& g = *GImGui;
			std::stringstream code;
			if (v_id == "")
				return "";

			ImGui::Text(v_label.c_str());
			code << "ImGui::Text(\"" << v_label << "\");";
			return code.str();
		}

		virtual std::string RenderFoot() override
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