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
	class Button : ImGuiElement
	{
	public:
		static inline std::unordered_map<Button*, std::vector<Button>> undo_stack;
		static inline std::unordered_map<Button*, std::vector<Button>> redo_stack;

		Button() {
			ImGuiContext& g = *GImGui;
			v_flags = ImGuiButtonFlags_None;
			v_property_flags = property_flags::disabled | property_flags::pos;
			v_size = ImVec2(0, 0);
			v_id = ("new button##" + RandomID()).c_str();
			v_label = "new button";
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_Button] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_colors[ImGuiCol_ButtonHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_colors[ImGuiCol_ButtonActive] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_styles[ImGuiStyleVar_ItemInnerSpacing] = g.Style.ItemInnerSpacing;
			v_styles[ImGuiStyleVar_ItemSpacing] = g.Style.ItemSpacing;
			v_styles[ImGuiStyleVar_ButtonTextAlign] = g.Style.ButtonTextAlign;
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

		virtual void Clone() override
		{
			igd::active_workspace->elements_buffer.push_back((ImGuiElement*)(new Button()));
			*igd::active_workspace->elements_buffer.back() = *this;
			igd::active_workspace->elements_buffer.back()->v_id = RandomID().c_str();
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
			if (v_size.type == Vec2Type::Absolute)
			{
				ImGui::Button(v_id.c_str(), v_size.value);
				code << "ImGui::Button(\"" << v_id << "\",  {" << v_size.value.x <<"," << v_size.value.y << "});";
			}
			else if (v_size.type == Vec2Type::Relative)
			{
				ImGui::Button(v_id.c_str(), { ContentRegionAvail.x * (v_size.value.x / 100),ContentRegionAvail.y * (v_size.value.y / 100) });
				code << "ImGui::Button(\"" << v_id << "\",  { " << ContentRegionString << ".x * "<<v_size.value.x / 100<<", " << ContentRegionString << ".y * " << v_size.value.y / 100 << "}); ";
			}
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
			GenerateStylesColorsJson(j, "button");
			return j;
		}
};
}