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
	class Selectable : ImGuiElement
	{
	public:
		static inline std::unordered_map<Selectable*, std::vector<Selectable>> undo_stack;
		static inline std::unordered_map<Selectable*, std::vector<Selectable>> redo_stack;
		static inline std::string json_identifier = "selectable";
		bool selected = false;
		Selectable() {
			v_type_id = (int)element_type::selectable;
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiSelectableFlags_None;
			v_property_flags = property_flags::disabled | property_flags::pos;
			v_size = ImVec2(0, 0);
			v_id = ("Selectable##" + RandomID()).c_str();
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_FrameBg] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			v_colors[ImGuiCol_FrameBgActive] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);
			v_colors[ImGuiCol_FrameBgHovered] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			v_custom_flags[ImGuiSelectableFlags_AllowDoubleClick] = "ImGuiSelectableFlags_AllowDoubleClick";
			v_custom_flags[ImGuiSelectableFlags_DontClosePopups] = "ImGuiSelectableFlags_DontClosePopups";
			v_custom_flags[ImGuiSelectableFlags_SpanAllColumns] = "ImGuiSelectableFlags_SpanAllColumns";
			v_custom_flags[ImGuiSelectableFlags_Disabled] = "ImGuiSelectableFlags_Disabled";
			v_custom_flags[ImGuiSelectableFlags_AllowItemOverlap] = "ImGuiSelectableFlags_AllowItemOverlap";
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_styles[ImGuiStyleVar_ItemInnerSpacing] = g.Style.ItemInnerSpacing;
			v_styles[ImGuiStyleVar_ItemSpacing] = g.Style.ItemSpacing;
			v_styles[ImGuiStyleVar_SelectableTextAlign] = g.Style.SelectableTextAlign;
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
			Selectable* new_element = new Selectable();
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
			std::stringstream code;
			code << this->GetWidthScript() << std::endl;
			code << "if (ImGui::Selectable(\"" << v_id << "\", false, " << this->buildFlagString() << ", " << this->GetSizeScript() << "))" << std::endl;
			code << "{" << std::endl << "\t//selectable clicked" << std::endl;
			code << "\t" << this->v_parent->GetIDForVariable() << " = \"" << v_id << "\";" << std::endl << "}";
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
			std::stringstream code;
			bool did_select = false;
			if (script_only)
				return ScriptInternal();
			if (v_id == "")
				return "";
			if (this->v_parent)
				selected = this->v_parent->v_label == this->v_id;
						
			did_select = ImGui::Selectable(v_id.c_str(), &selected, v_flags, this->GetSize());
			if (did_select && this->v_parent)
				this->v_parent->v_label = this->v_id;
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
			std::cout << "Adding a Selectable" << std::endl;
			igd::Selectable* b = new igd::Selectable();
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