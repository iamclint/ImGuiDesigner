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
		static inline std::unordered_map<ChildWindow*, std::vector<ChildWindow>> undo_stack;
		static inline std::unordered_map<ChildWindow*, std::vector<ChildWindow>> redo_stack;
		ChildWindow() {
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::disabled | property_flags::border;
			v_size = ImVec2(0, 0);
			v_id = ("child window##" + RandomID(10)).c_str();
			v_label = "";
			v_border = true;
			v_can_have_children = true;
			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_Button] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_colors[ImGuiCol_ButtonHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_colors[ImGuiCol_ButtonActive] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			v_colors[ImGuiCol_Border] = ImGui::GetStyleColorVec4(ImGuiCol_Border);
			v_colors[ImGuiCol_BorderShadow] = ImGui::GetStyleColorVec4(ImGuiCol_BorderShadow);
			v_colors[ImGuiCol_ChildBg] = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
			v_colors[ImGuiCol_CheckMark] = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
			v_colors[ImGuiCol_FrameBg] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			v_colors[ImGuiCol_FrameBgActive] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);
			v_colors[ImGuiCol_FrameBgHovered] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			v_colors[ImGuiCol_Header] = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			v_colors[ImGuiCol_HeaderActive] = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
			v_colors[ImGuiCol_HeaderHovered] = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			v_colors[ImGuiCol_NavHighlight] = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			v_colors[ImGuiCol_PlotHistogram] = ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram);
			v_colors[ImGuiCol_PlotHistogramHovered] = ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogramHovered);
			v_colors[ImGuiCol_PlotLines] = ImGui::GetStyleColorVec4(ImGuiCol_PlotLines);
			v_colors[ImGuiCol_PlotLinesHovered] = ImGui::GetStyleColorVec4(ImGuiCol_PlotLinesHovered);
			v_colors[ImGuiCol_ScrollbarBg] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarBg);
			v_colors[ImGuiCol_ScrollbarGrab] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrab);
			v_colors[ImGuiCol_ScrollbarGrabActive] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);
			v_colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabHovered);
			v_colors[ImGuiCol_SliderGrab] = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab);
			v_colors[ImGuiCol_SliderGrabActive] = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrabActive);
			v_colors[ImGuiCol_Tab] = ImGui::GetStyleColorVec4(ImGuiCol_Tab);
			v_colors[ImGuiCol_TabActive] = ImGui::GetStyleColorVec4(ImGuiCol_TabActive);
			v_colors[ImGuiCol_TabHovered] = ImGui::GetStyleColorVec4(ImGuiCol_TabHovered);
			v_colors[ImGuiCol_TabUnfocused] = ImGui::GetStyleColorVec4(ImGuiCol_TabUnfocused);
			v_colors[ImGuiCol_TabUnfocusedActive] = ImGui::GetStyleColorVec4(ImGuiCol_TabUnfocusedActive);
			v_colors[ImGuiCol_ResizeGrip] = ImGui::GetStyleColorVec4(ImGuiCol_ResizeGrip);
			v_colors[ImGuiCol_ResizeGripActive] = ImGui::GetStyleColorVec4(ImGuiCol_ResizeGripActive);
			v_colors[ImGuiCol_ResizeGripHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ResizeGripHovered);
			v_colors[ImGuiCol_Separator] = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
			v_colors[ImGuiCol_SeparatorActive] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive);
			v_colors[ImGuiCol_SeparatorHovered] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered);
			v_colors[ImGuiCol_TextSelectedBg] = ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg);
			v_colors[ImGuiCol_TextDisabled] = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
			v_colors[ImGuiCol_TableBorderLight] = ImGui::GetStyleColorVec4(ImGuiCol_TableBorderLight);
			v_colors[ImGuiCol_TableBorderStrong] = ImGui::GetStyleColorVec4(ImGuiCol_TableBorderStrong);
			v_colors[ImGuiCol_TableHeaderBg] = ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg);
			v_colors[ImGuiCol_TableRowBg] = ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg);
			v_colors[ImGuiCol_TableRowBgAlt] = ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt);
			v_colors[ImGuiCol_DragDropTarget] = ImGui::GetStyleColorVec4(ImGuiCol_DragDropTarget);
			v_colors[ImGuiCol_NavWindowingHighlight] = ImGui::GetStyleColorVec4(ImGuiCol_NavWindowingHighlight);
			v_colors[ImGuiCol_NavWindowingDimBg] = ImGui::GetStyleColorVec4(ImGuiCol_NavWindowingDimBg);
			
			
			v_styles[ImGuiStyleVar_ChildBorderSize] = g.Style.ChildBorderSize;
			v_styles[ImGuiStyleVar_ChildRounding] = g.Style.ChildRounding;
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

		virtual void Clone() override
		{
			igd::active_workspace->elements_buffer.push_back((ImGuiElement*)(new ChildWindow()));
			*igd::active_workspace->elements_buffer.back() = *this;
			igd::active_workspace->elements_buffer.back()->v_id = RandomID(10).c_str();
		}
		
		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{
		
		}
	
		virtual std::string RenderHead() override
		{
			if (v_id == "")
				return "";
			std::stringstream code_out;
			ImGuiContext& g = *GImGui;

			if (v_size.type == Vec2Type::Absolute)
			{
				ImGui::BeginChild(v_id.c_str(), v_size.value, v_border, v_flags);
				code_out << "ImGui::BeginChild(\"" << v_id << "\", {" << v_size.value.x << "," << v_size.value.y << "}, " << v_border << ", " << v_flags << ");";
			}
			else if (v_size.type == Vec2Type::Relative)
			{
				ImGui::BeginChild(v_id.c_str(), { ContentRegionAvail.x * (v_size.value.x / 100),ContentRegionAvail.y * (v_size.value.y / 100) }, v_border, v_flags);
				code_out << "ImGui::BeginChild(\"" << v_id << "\", { " << ContentRegionString << ".x * " << v_size.value.x / 100 << ", " << ContentRegionString << ".y * " << v_size.value.y / 100 << " }, " << v_border << ", " << v_flags << "); ";
			}
			return code_out.str();
		}
		virtual std::string RenderInternal() override
		{
			//iterate all children handled by imguielement cpp
			return "";
		}
		virtual std::string RenderFoot() override
		{
			if (v_id == "")
				return "";
			ImGui::EndChild();
			return "ImGui::EndChild();";
		}
		virtual void FromJSON(nlohmann::json data) override
		{
			StylesColorsFromJson(data);
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			GenerateStylesColorsJson(j, "child window");
			return j;
		}
		
	};
}