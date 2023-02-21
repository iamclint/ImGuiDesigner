#pragma once
#include "ImGuiElement.h"
#include <string>
#include <map>
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiDesigner.h"
#include <iostream>
namespace igd
{
	class Button : ImGuiElement
	{
	public:
		static inline std::unordered_map<Button*, std::vector<Button>> undo_stack;
		static inline std::unordered_map<Button*, std::vector<Button>> redo_stack;
		int color_pops;
		Button() {
			color_pops = 0;
			v_flags = ImGuiButtonFlags_None;
			v_property_flags = property_flags::label | property_flags::color_background | property_flags::color_background_active | property_flags::color_background_hovered;
			v_size = ImVec2(0, 0);
			v_id = ("new button##" + RandomID(10)).c_str();
			v_label = "new button";
			v_foreground = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_background = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_background_hovered = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_background_active = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			v_can_have_children = false;
			PushUndo();
		}

		virtual void Undo() override
		{
			if (undo_stack[this].size() > 0)
			{
				std::cout << "undo stack size button: " << undo_stack[this].size() << std::endl;
				redo_stack[this].push_back(*this);
				if (undo_stack[this].size() > 1)
					undo_stack[this].pop_back();
				*this = undo_stack[this].back();
				if (undo_stack[this].size() > 1)//don't remove the initial stack 
					undo_stack[this].pop_back();
			}
		}

		virtual void PushUndo() override
		{
			did_move = false;
			did_resize = false;
			undo_stack[this].push_back(*this);
			igd::active_workspace->PushUndo(this);
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
			color_pops = 0;

			ImGui::PushStyleColor(ImGuiCol_Text, v_foreground.Value);
			color_pops++;

			ImGui::PushStyleColor(ImGuiCol_Button, v_background.Value);
			color_pops++;

			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, v_background_hovered.Value);
			color_pops++;

			ImGui::PushStyleColor(ImGuiCol_ButtonActive, v_background_active.Value);
			color_pops++;
		}
		virtual void RenderInternal() override
		{
			ImGui::Button((v_label + "##" + v_id).c_str(), v_size);
		}
		virtual void RenderFoot() override
		{
			if (color_pops)
				ImGui::PopStyleColor(color_pops);
		}
	};

}