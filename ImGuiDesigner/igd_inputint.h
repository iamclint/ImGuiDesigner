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
	class InputInt : ImGuiElement
	{
	public:

		static inline std::unordered_map<InputInt*, std::vector<InputInt>> undo_stack;
		static inline std::unordered_map<InputInt*, std::vector<InputInt>> redo_stack;
		static inline std::string json_identifier = "inputint";
		int input_data;
		int step;
		int step_fast;
		InputInt() {
			v_type_id = (int)element_type::inputint;
			input_data = 0;
			step = 1;
			step_fast = 100;
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::label | property_flags::disabled;
			v_size = ImVec2(0, 0);
			v_id = ("InputInt##" + RandomID()).c_str();
			v_label = "";

			v_custom_flags[ImGuiInputTextFlags_CharsNoBlank] = "ImGuiInputTextFlags_CharsNoBlank";
			v_custom_flags[ImGuiInputTextFlags_AutoSelectAll] = "ImGuiInputTextFlags_AutoSelectAll";
			v_custom_flags[ImGuiInputTextFlags_EnterReturnsTrue] = "ImGuiInputTextFlags_EnterReturnsTrue";
			v_custom_flags[ImGuiInputTextFlags_CallbackCompletion] = "ImGuiInputTextFlags_CallbackCompletion";
			v_custom_flags[ImGuiInputTextFlags_CallbackHistory] = "ImGuiInputTextFlags_CallbackHistory";
			v_custom_flags[ImGuiInputTextFlags_CallbackAlways] = "ImGuiInputTextFlags_CallbackAlways";
			v_custom_flags[ImGuiInputTextFlags_CallbackCharFilter] = "ImGuiInputTextFlags_CallbackCharFilter";
			v_custom_flags[ImGuiInputTextFlags_NoHorizontalScroll] = "ImGuiInputTextFlags_NoHorizontalScroll";
			v_custom_flags[ImGuiInputTextFlags_AlwaysOverwrite] = "ImGuiInputTextFlags_AlwaysOverwrite";
			v_custom_flags[ImGuiInputTextFlags_ReadOnly] = "ImGuiInputTextFlags_ReadOnly";
			v_custom_flags[ImGuiInputTextFlags_NoUndoRedo] = "ImGuiInputTextFlags_NoUndoRedo";
			v_custom_flags[ImGuiInputTextFlags_CharsScientific] = "ImGuiInputTextFlags_CharsScientific";
			v_custom_flags[ImGuiInputTextFlags_CallbackResize] = "ImGuiInputTextFlags_CallbackResize";
			v_custom_flags[ImGuiInputTextFlags_CallbackEdit] = "ImGuiInputTextFlags_CallbackEdit";

			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_FrameBg] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			v_colors[ImGuiCol_FrameBgActive] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);
			v_colors[ImGuiCol_FrameBgHovered] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			v_colors[ImGuiCol_TextSelectedBg] = ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg);
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
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
			InputInt* new_element = new InputInt();
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
			igd::properties->PropertyLabel("value:");
			ImGui::InputInt("##properties_value", &input_data);
			igd::properties->PropertyLabel("step:");
			ImGui::InputInt("##properties_step", &step);
			igd::properties->PropertyLabel("step fast:");
			ImGui::InputInt("##properties_step_fast", &step_fast);
		}

		virtual std::string RenderHead(bool script_only) override
		{
			ImGuiContext& g = *GImGui;
			return "";
		}

		virtual std::string RenderInternal(bool script_only) override
		{
			ImGuiContext& g = *GImGui;
			std::stringstream code;
			code << "static float " << this->GetIDForVariable() << ";" << std::endl;
			if (v_size.type == Vec2Type::Absolute && v_size.value.x != 0)
			{
				if (!script_only)
					ImGui::SetNextItemWidth(v_size.value.x);
				code << "ImGui::SetNextItemWidth(" << igd::fString(v_size.value.x) << ");" << std::endl;
			}
			else if (v_size.type == Vec2Type::Relative && v_size.value.x != 0)
			{
				if (!script_only)
					ImGui::SetNextItemWidth(ContentRegionAvail.x * (v_size.value.x / 100));
				code << "ImGui::SetNextItemWidth(ContentRegionAvail.x * " << igd::fString(v_size.value.x / 100.f) << ");" << std::endl;
			}
			if (!script_only)
				ImGui::InputInt((v_label + "##" + v_id).c_str(), &input_data, step, step_fast, v_flags);
			code << "ImGui::InputInt(\"" << v_label << "##" << v_id << "\", &" << this->GetIDForVariable() << ", " << igd::fString(step) << ", " << igd::fString(step_fast) << ", " << this->buildFlagString() << ");";
			return code.str();
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
			std::cout << "Adding a InputInt" << std::endl;
			igd::InputInt* b = new igd::InputInt();
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