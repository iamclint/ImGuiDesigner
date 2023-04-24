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
	class InputText : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "inputtext";
		std::string input_data;
		std::string default_txt;
		InputText() {
			v_icon = igd::textures.images["input"];
			v_tooltip = "Input Text";
			v_type_id = (int)element_type::inputtext;
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiInputTextFlags_None;
			v_property_flags = property_flags::pos | property_flags::label | property_flags::disabled | property_flags::has_variable;
			v_variable_name = json_identifier + "_" + RandomID();
			v_size = ImVec2(0, 0);
			v_id = ("InputText##" + RandomID()).c_str();
			v_label = "";
			default_txt = "Enter text here";
			v_custom_flags[ImGuiInputTextFlags_AllowTabInput] = "ImGuiInputTextFlags_AllowTabInput";
			v_custom_flags[ImGuiInputTextFlags_CharsDecimal] = "ImGuiInputTextFlags_CharsDecimal";
			v_custom_flags[ImGuiInputTextFlags_CharsHexadecimal] = "ImGuiInputTextFlags_CharsHexadecimal";
			v_custom_flags[ImGuiInputTextFlags_CharsUppercase] = "ImGuiInputTextFlags_CharsUppercase";
			v_custom_flags[ImGuiInputTextFlags_CharsNoBlank] = "ImGuiInputTextFlags_CharsNoBlank";
			v_custom_flags[ImGuiInputTextFlags_AutoSelectAll] = "ImGuiInputTextFlags_AutoSelectAll";
			v_custom_flags[ImGuiInputTextFlags_EnterReturnsTrue] = "ImGuiInputTextFlags_EnterReturnsTrue";
			v_custom_flags[ImGuiInputTextFlags_CallbackCompletion] = "ImGuiInputTextFlags_CallbackCompletion";
			v_custom_flags[ImGuiInputTextFlags_CallbackHistory] = "ImGuiInputTextFlags_CallbackHistory";
			v_custom_flags[ImGuiInputTextFlags_CallbackAlways] = "ImGuiInputTextFlags_CallbackAlways";
			v_custom_flags[ImGuiInputTextFlags_CallbackCharFilter] = "ImGuiInputTextFlags_CallbackCharFilter";
			v_custom_flags[ImGuiInputTextFlags_CtrlEnterForNewLine] = "ImGuiInputTextFlags_CtrlEnterForNewLine";
			v_custom_flags[ImGuiInputTextFlags_NoHorizontalScroll] = "ImGuiInputTextFlags_NoHorizontalScroll";
			v_custom_flags[ImGuiInputTextFlags_AlwaysOverwrite] = "ImGuiInputTextFlags_AlwaysOverwrite";
			v_custom_flags[ImGuiInputTextFlags_ReadOnly] = "ImGuiInputTextFlags_ReadOnly";
			v_custom_flags[ImGuiInputTextFlags_Password] = "ImGuiInputTextFlags_Password";
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
		}
	
		virtual ImGuiElement* Clone() override
		{
			InputText* new_element = new InputText();
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
			igd::properties->PropertyLabel("Default Text");
			ImGui::PushItemWidth(120);
			ImGui::InputText("##property_default_text", &default_txt);
		}
		std::string ScriptHead() {
			return "";
		};

		std::string GetVariableCode()
		{
			std::stringstream code_out;
			code_out << "std::string " << v_variable_name << " = \"" << input_data << "\";";
			return code_out.str();
		}

		std::string ScriptInternal() {
			std::stringstream code;
			code << igd::script::GetWidthScript(this) << std::endl;
			code << "ImGui::InputText(\"" << v_label << "##" << v_id << "\", &" << v_variable_name << ", " << igd::script::BuildFlagString(this) << ");" << std::endl;
			code << "if (" << v_variable_name << ".size() == 0)" << std::endl;
			code << "\tImGui::SetDefaultText(\"" << default_txt << "\");" << std::endl;
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
			ImGuiContext& g = *GImGui;
			if (script_only)
				return ScriptInternal();

			this->SetNextWidth();
			ImGui::InputText((v_label + "##" + v_id).c_str(), &input_data, v_flags);
			if (input_data.size()==0)
				ImGui::SetDefaultText(default_txt);
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
			std::cout << "Adding a InputText" << std::endl;
			igd::InputText* b = new igd::InputText();
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