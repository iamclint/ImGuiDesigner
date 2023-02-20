#pragma once
#include "ImGuiElement.h"
#include <string>
#include <vector>
#include "Workspace.h"
namespace igd
{


	class ChildWindow : ImGuiElement
	{
	public:
		int color_pops;
		std::vector<ChildWindow> undo_stack;
		ChildWindow() {
			color_pops = 0;
			v_flags = ImGuiButtonFlags_None;
			v_size = ImVec2(0, 0);
			v_id = ("child window##" + RandomID(10)).c_str();
			v_label = "";
			v_foreground = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_background = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
			v_border = true;
			v_can_have_children = true;
			v_property_flags = property_flags::color_background | property_flags::border;
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{

		}

		virtual void Undo() override
		{
			*this = undo_stack.back();
			undo_stack.pop_back();
		}

		virtual void PushUndo() override
		{
			undo_stack.push_back(*this);
			igd::active_workspace->PushUndo(this);
		}
		
		virtual void RenderHead() override
		{
			color_pops = 0;
			if (v_foreground.Value.w != 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, v_foreground.Value);
				color_pops++;
			}
			if (v_background.Value.w != 0)
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, v_background.Value);
				color_pops++;
			}
			ImGui::BeginChild(v_id.c_str(), v_size, v_border, v_flags);
		}
		virtual void RenderInternal() override
		{
			//iterate all children handled by imguielement cpp

		}
		virtual void RenderFoot() override
		{
			ImGui::EndChild();
			if (color_pops)
				ImGui::PopStyleColor(color_pops);
		}

		virtual void Clone() override
		{
			igd::active_workspace->elements_buffer.push_back((ImGuiElement*)(new ChildWindow()));
			igd::active_workspace->elements_buffer.back()->v_background = this->v_background;
			igd::active_workspace->elements_buffer.back()->v_foreground = this->v_foreground;
			igd::active_workspace->elements_buffer.back()->v_flags = this->v_flags;
			igd::active_workspace->elements_buffer.back()->v_label = this->v_label;
			igd::active_workspace->elements_buffer.back()->v_size = this->v_size;
		}
	};
}