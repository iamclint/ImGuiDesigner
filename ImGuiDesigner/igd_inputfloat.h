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
	class InputFloat : ImGuiElement
	{
	public:

		static inline std::unordered_map<InputFloat*, std::vector<InputFloat>> undo_stack;
		static inline std::unordered_map<InputFloat*, std::vector<InputFloat>> redo_stack;
		static inline std::string json_identifier = "inputfloat";
		float input_data;
		float step;
		float step_fast;
		std::string format;
		InputFloat() {
			input_data = 0;
			format = "%.3f";
			step = 0.f;
			step_fast = 0.f;
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::label | property_flags::disabled;
			v_size = ImVec2(0, 0);
			v_id = ("InputFloat##" + RandomID()).c_str();
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
			InputFloat* new_element = new InputFloat();
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
			ImGui::InputFloat("##properties_value", &input_data);
			igd::properties->PropertyLabel("step:");
			ImGui::InputFloat("##properties_step", &step);
			igd::properties->PropertyLabel("step fast:");
			ImGui::InputFloat("##properties_step_fast", &step_fast);
			igd::properties->PropertyLabel("format:");
			ImGui::InputText("##properties_format", &format);
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
			code << "static float " << this->GetIDForVariable() << ";" << std::endl;
			if (v_size.type == Vec2Type::Absolute && v_size.value.x != 0)
			{
				ImGui::SetNextItemWidth(v_size.value.x);
				code << "ImGui::SetNextItemWidth(" << igd::fString(v_size.value.x) << ");" << std::endl;
			}
			else if (v_size.type == Vec2Type::Relative && v_size.value.x != 0)
			{
				ImGui::SetNextItemWidth(ContentRegionAvail.x * (v_size.value.x / 100));
				code << "ImGui::SetNextItemWidth(ContentRegionAvail.x * " << igd::fString(v_size.value.x / 100.f) << ");" << std::endl;
			}
			ImGui::InputFloat((v_label + "##" + v_id).c_str(), &input_data, step, step_fast, format.c_str(), v_flags);
			code << "ImGui::InputFloat(\"" << v_label << "##" << v_id << "\", &" << this->GetIDForVariable() << ", " << igd::fString(step) << ", " << igd::fString(step_fast) << ", \"" << format << "\", " << this->buildFlagString() << ");";
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
			std::cout << "Adding a InputFloat" << std::endl;
			igd::InputFloat* b = new igd::InputFloat();
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