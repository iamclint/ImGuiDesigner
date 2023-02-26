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
			v_flags = ImGuiButtonFlags_None;
			v_property_flags = property_flags::label | property_flags::color_foreground | property_flags::color_background | property_flags::color_background_active | property_flags::color_background_hovered | property_flags::disabled;
			v_size = ImVec2(0, 0);
			v_id = ("new button##" + RandomID(10)).c_str();
			v_label = "new button";
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_Button] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_colors[ImGuiCol_ButtonHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_colors[ImGuiCol_ButtonActive] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
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
			igd::active_workspace->elements_buffer.back()->v_id = RandomID(10).c_str();
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{

		}

		virtual void RenderHead() override
		{
			ImGuiContext& g = *GImGui;

		}

		virtual void RenderInternal() override
		{
			igd::active_workspace->code << "ImGui::Button(\"" << v_label << "\", ImVec2(" << v_size.x << ", " << v_size.y << "));" << std::endl;
			ImGui::Button((v_label + "##" + v_id).c_str(), v_size);
		}

		virtual void RenderFoot() override
		{

		
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