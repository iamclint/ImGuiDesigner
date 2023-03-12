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
		static inline std::unordered_map<Combo*, std::vector<Combo>> undo_stack;
		static inline std::unordered_map<Combo*, std::vector<Combo>> redo_stack;
		static inline std::string json_identifier = "combo";
		Combo() {
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::disabled | property_flags::border | property_flags::label;
			v_size = ImVec2(0, 0);
			v_id = ("combo box##" + RandomID()).c_str();
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
			Combo* new_element = new Combo();
			*new_element = *this;
			std::string new_id = this->v_id;
			if (new_id.find("##") != std::string::npos)
				new_id = new_id.substr(0, new_id.find("##") + 2);

			if (active_workspace->active_element->v_can_have_children)
				new_element->v_parent = active_workspace->active_element;
			else
				new_element->v_parent = active_workspace->active_element->v_parent;

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

		virtual std::string RenderHead(bool script_only) override
		{
			if (v_id == "")
				return "";
			std::stringstream code_out;
			ImGuiContext& g = *GImGui;
			code_out << "static std::string " << this->GetIDForVariable() << " = \"\";" << std::endl;
			if (v_size.type == Vec2Type::Absolute && v_size.value.x != 0)
			{
				code_out << "ImGui::SetNextItemWidth(" << v_size.value.x << ");" << std::endl;
				if (!script_only)
					ImGui::SetNextItemWidth(v_size.value.x);
			}
			else if (v_size.type == Vec2Type::Relative && v_size.value.x != 0)
			{
				code_out << "ImGui::SetNextItemWidth(" << ContentRegionString << ".x * " << igd::fString(v_size.value.x / 100) << ");" << std::endl;
				if (!script_only)
					ImGui::SetNextItemWidth(ContentRegionAvail.x * (v_size.value.x / 100));
			}
			v_is_open = ImGui::BeginCombo(v_id.c_str(), v_label.c_str(), v_flags);
			code_out << "if (ImGui::BeginCombo(\"" << v_id << "\"," << this->GetIDForVariable() << ".c_str(), " << this->buildFlagString() << "))";
			return code_out.str();
		}
		virtual std::string RenderInternal(bool script_only) override
		{
			//iterate all children handled by imguielement cpp
			std::stringstream code_out;
			code_out << "ImGui::EndCombo();";
			return code_out.str();
		}
		virtual std::string RenderFoot(bool script_only) override
		{
			if (v_id == "")
				return "";
			if (v_is_open && !script_only)
				ImGui::EndCombo();
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