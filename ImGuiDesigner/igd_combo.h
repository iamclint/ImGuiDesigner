#pragma once
#include "ImGuiElement.h"
#include <string>
#include <vector>
#include "Workspace.h"

namespace igd
{
	class Combo : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "combo";
		Combo() {
			v_icon = igd::textures.images["combo"];
			v_tooltip = "Combo Box";
			v_type_id = (int)element_type::combo;
			v_can_contain_own_type = false;
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::disabled | property_flags::border | property_flags::label | property_flags::has_variable;
			v_variable_name = json_identifier + "_" + RandomID();
			v_size = ImVec2(0, 0);
			v_id = ("Combo box##" + RandomID()).c_str();
			v_label = "";
			v_border = true;
			v_can_have_children = true;
			v_requires_open = true;
			v_is_open = false;
			//available child window flags
			v_custom_flags[ImGuiComboFlags_CustomPreview] = "ImGuiComboFlags_CustomPreview";
			v_custom_flags[ImGuiComboFlags_PopupAlignLeft] = "ImGuiComboFlags_PopupAlignLeft";
			v_custom_flags[ImGuiComboFlags_HeightSmall] = "ImGuiComboFlags_HeightSmall";
			v_custom_flags[ImGuiComboFlags_HeightRegular] = "ImGuiComboFlags_HeightRegular";
			v_custom_flags[ImGuiComboFlags_HeightLarge] = "ImGuiComboFlags_HeightLarge";
			v_custom_flags[ImGuiComboFlags_HeightLargest] = "ImGuiComboFlags_HeightLargest";
			v_custom_flags[ImGuiComboFlags_NoArrowButton] = "ImGuiComboFlags_NoArrowButton";
			v_custom_flags[ImGuiComboFlags_NoPreview] = "ImGuiComboFlags_NoPreview";
			v_custom_flags[ImGuiComboFlags_HeightMask_] = "ImGuiComboFlags_HeightMask_";

			v_custom_flag_groups[ImGuiComboFlags_HeightMask_] = true;
			v_custom_flag_groups[ImGuiWindowFlags_NoInputs] = true;

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
			v_colors[ImGuiCol_TextDisabled] = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);

			v_styles[ImGuiStyleVar_ButtonTextAlign] = g.Style.ButtonTextAlign;
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
			Combo* new_element = new Combo();
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


		std::string GetVariableCode()
		{
			std::stringstream code_out;
			code_out << "std::string " << v_variable_name << " = \"\";";
			return code_out.str();
		}


		std::string ScriptHead() { 
			std::stringstream code;
			code << igd::script::GetWidthScript(this)  << std::endl;
			code << "if (ImGui::BeginCombo(\"" << v_id << "\"," << v_variable_name << ".c_str(), " << igd::script::BuildFlagString(this) << "))";
			return code.str(); 
		};
		std::string ScriptInternal() {
			return "ImGui::EndCombo();";
		};

		std::string ScriptFoot() { return ""; };

		virtual std::string RenderHead(bool script_only) override
		{
			if (script_only)
				return ScriptHead();
			
			if (v_id == "")
				return "";
			ImGuiContext& g = *GImGui;
			this->SetNextWidth();
			if (igd::active_workspace->interaction_mode == InteractionMode::designer)
			{
				if (!v_disabled)
					GImGui->Style.DisabledAlpha = 1.0f;
				ImGui::BeginDisabled();
			}
			v_is_open = ImGui::BeginCombo(v_id.c_str(), v_label.c_str(), v_flags);
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
				ImGui::EndCombo();

			if (igd::active_workspace->interaction_mode == InteractionMode::designer && !script_only)
				ImGui::EndDisabled();
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
			std::cout << "Combo found" << std::endl;
			igd::Combo* b = new igd::Combo();
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