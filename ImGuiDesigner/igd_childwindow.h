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
			v_flags = 0;
			v_property_flags =  property_flags::color_background | property_flags::disabled | property_flags::border;
			v_size = ImVec2(0, 0);
			v_id = ("child window##" + RandomID(10)).c_str();
			v_label = "";
			v_foreground = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_background = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
			v_border = true;
			v_can_have_children = true;
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
			igd::properties->PropertyLabel("Rounding");
			ImGui::SliderFloat("##property_rounding", &this->v_ImGuiStyleVar_ChildRounding, 0, 36);
			igd::properties->PropertyLabel("Frame Rounding");
			ImGui::SliderFloat("##property_frame_rounding", &this->v_ImGuiStyleVar_FrameRounding, 0, 36);
			
			igd::properties->PropertyLabel("Border Size");
			ImGui::SliderFloat("##property_border_size", &this->v_ImGuiStyleVar_ChildBorderSize, 0, 36);
			igd::properties->PropertyLabel("Indent Spacing");
			ImGui::SliderFloat("##property_indent_spacing", &this->v_ImGuiStyleVar_IndentSpacing, 0, 36);
			igd::properties->PropertyLabel("Scrollbar Size");
			ImGui::SliderFloat("##property_scrollbar_size", &this->v_ImGuiStyleVar_ScrollbarSize, 0, 36);
			//igd::properties->PropertyLabel("Grabber Min");
			//ImGui::SliderFloat("##property_Grabber_Min", &this->v_ImGuiStyleVar_GrabMinSize, 0, 36);
			//igd::properties->PropertyLabel("Grabber Rounding");
			//ImGui::SliderFloat("##property_Grabber_Rounding", &this->v_ImGuiStyleVar_GrabRounding, 0, 36);
			//igd::properties->PropertyLabel("Layout Align");
			//ImGui::InputFloat2("##property_Layout_Align", (float*)&this->v_ImGuiStyleVar_LayoutAlign);
			igd::properties->PropertyLabel("Frame Padding");
			ImGui::InputFloat2("##property_frame_padding", (float*)&this->v_ImGuiStyleVar_FramePadding);
			igd::properties->PropertyLabel("Item Spacing");
			ImGui::InputFloat2("##property_item_spacing", (float*)&this->v_ImGuiStyleVar_ItemSpacing);
			igd::properties->PropertyLabel("Item Inner Spacing");
			ImGui::InputFloat2("##property_item_inner_spacing", (float*)&this->v_ImGuiStyleVar_ItemInnerSpacing);
		}
	
		virtual void RenderHead() override
		{
			ImGuiContext& g = *GImGui;
			this->PushStyleColor(ImGuiCol_Text, v_foreground.Value);
			this->PushStyleColor(ImGuiCol_ChildBg, v_background.Value);
			
			if (v_ImGuiStyleVar_ChildRounding!=0)
				this->PushStyleVar(ImGuiStyleVar_ChildRounding, v_ImGuiStyleVar_ChildRounding);
			if (v_ImGuiStyleVar_ChildBorderSize != 0)
				this->PushStyleVar(ImGuiStyleVar_ChildBorderSize, v_ImGuiStyleVar_ChildBorderSize);
			if (v_ImGuiStyleVar_IndentSpacing != 0)
				this->PushStyleVar(ImGuiStyleVar_IndentSpacing, v_ImGuiStyleVar_IndentSpacing);
			if (v_ImGuiStyleVar_ScrollbarSize != 0)
				this->PushStyleVar(ImGuiStyleVar_ScrollbarSize, v_ImGuiStyleVar_ScrollbarSize);
			if (v_ImGuiStyleVar_GrabMinSize != 0)
				this->PushStyleVar(ImGuiStyleVar_GrabMinSize, v_ImGuiStyleVar_GrabMinSize);
			if (v_ImGuiStyleVar_GrabRounding != 0)
				this->PushStyleVar(ImGuiStyleVar_GrabRounding, v_ImGuiStyleVar_GrabRounding);
			//if (v_ImGuiStyleVar_LayoutAlign != 0)
			//	this->PushStyleVar(ImGuiStyleVar_LayoutAlign, v_ImGuiStyleVar_LayoutAlign);
			if (v_ImGuiStyleVar_FramePadding.x != 0 || v_ImGuiStyleVar_FramePadding.y != 0)
				this->PushStyleVar(ImGuiStyleVar_FramePadding, v_ImGuiStyleVar_FramePadding);
			if (v_ImGuiStyleVar_ItemSpacing.x != 0 || v_ImGuiStyleVar_ItemSpacing.y != 0)
				this->PushStyleVar(ImGuiStyleVar_ItemSpacing, v_ImGuiStyleVar_ItemSpacing);
			if (v_ImGuiStyleVar_ItemInnerSpacing.x != 0 || v_ImGuiStyleVar_ItemInnerSpacing.y != 0)
				this->PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, v_ImGuiStyleVar_ItemInnerSpacing);
						
			ImGui::BeginChild(v_id.c_str(), v_size, v_border, v_flags);
		}
		virtual void RenderInternal() override
		{
			//iterate all children handled by imguielement cpp

		}
		virtual void RenderFoot() override
		{
			ImGui::EndChild();
		}
		virtual void FromJSON(nlohmann::json data) override
		{
			if (this->v_parent)
				v_pos = ImVec2(data["pos"][0], data["pos"][1]);
			v_id = data["id"].get<std::string>() + "##" + RandomID(10);
			v_flags = data["flags"];
			v_size = ImVec2(data["size"][0], data["size"][1]);
			v_label = data["label"].get<std::string>().c_str();
			v_foreground = ImVec4(data["foreground"][0], data["foreground"][1], data["foreground"][2], data["foreground"][3]);
			v_background = ImVec4(data["background"][0], data["background"][1], data["background"][2], data["background"][3]);
			v_border = data["border"];
			v_property_flags = data["property_flags"];
			v_disabled = data["disabled"];
			v_ImGuiStyleVar_ChildRounding = data["v_ImGuiStyleVar_ChildRounding"];
			v_ImGuiStyleVar_ChildBorderSize = data["v_ImGuiStyleVar_ChildBorderSize"];
			v_ImGuiStyleVar_FramePadding = ImVec2(data["v_ImGuiStyleVar_FramePadding"][0], data["v_ImGuiStyleVar_FramePadding"][1]);
			v_ImGuiStyleVar_ItemSpacing = ImVec2(data["v_ImGuiStyleVar_ItemSpacing"][0], data["v_ImGuiStyleVar_ItemSpacing"][1]);
			v_ImGuiStyleVar_ItemInnerSpacing = ImVec2(data["v_ImGuiStyleVar_ItemInnerSpacing"][0], data["v_ImGuiStyleVar_ItemInnerSpacing"][1]);
			v_ImGuiStyleVar_IndentSpacing = data["v_ImGuiStyleVar_IndentSpacing"];
			v_ImGuiStyleVar_ScrollbarSize = data["v_ImGuiStyleVar_ScrollbarSize"];
			//v_ImGuiStyleVar_GrabMinSize = data["v_ImGuiStyleVar_GrabMinSize"];
			//v_ImGuiStyleVar_GrabRounding = data["v_ImGuiStyleVar_GrabRounding"];
			//v_ImGuiStyleVar_LayoutAlign = data["v_ImGuiStyleVar_LayoutAlign"];
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			j["type"] = "child window";
			int pound_pos = v_id.find("#");
			if (pound_pos != std::string::npos)
				j["id"] = v_id.substr(0, pound_pos);
			else
				j["id"] = v_id;
			j["pos"] = { this->v_pos.x,this->v_pos.y };
			j["flags"] = v_flags;
			j["label"] = v_label;
			j["size"] = { v_size.x, v_size.y };
			j["flags"] = v_flags;
			j["foreground"] = { v_foreground.Value.x, v_foreground.Value.y, v_foreground.Value.z, v_foreground.Value.w };
			j["background"] = { v_background.Value.x, v_background.Value.y, v_background.Value.z, v_background.Value.w };
			j["border"] = v_border;
			j["disabled"] = v_disabled;
			j["property_flags"] = v_property_flags;
			j["v_ImGuiStyleVar_ChildRounding"] = v_ImGuiStyleVar_ChildRounding;
			j["v_ImGuiStyleVar_ChildBorderSize"] = v_ImGuiStyleVar_ChildBorderSize;
			j["v_ImGuiStyleVar_FramePadding"] = { v_ImGuiStyleVar_FramePadding.x,v_ImGuiStyleVar_FramePadding.y };
			j["v_ImGuiStyleVar_FrameRounding"] = v_ImGuiStyleVar_FrameRounding;
			j["v_ImGuiStyleVar_FrameBorderSize"] = v_ImGuiStyleVar_FrameBorderSize;
			j["v_ImGuiStyleVar_ItemSpacing"] = { v_ImGuiStyleVar_ItemSpacing.x,v_ImGuiStyleVar_ItemSpacing.y };
			j["v_ImGuiStyleVar_ItemInnerSpacing"] = { v_ImGuiStyleVar_ItemInnerSpacing.x, v_ImGuiStyleVar_ItemInnerSpacing.y };
			j["v_ImGuiStyleVar_IndentSpacing"] = v_ImGuiStyleVar_IndentSpacing;
			j["v_ImGuiStyleVar_ScrollbarSize"] = v_ImGuiStyleVar_ScrollbarSize;
			j["v_ImGuiStyleVar_ScrollbarRounding"] = v_ImGuiStyleVar_ScrollbarRounding;

		//	j["v_ImGuiStyleVar_GrabMinSize"] = v_ImGuiStyleVar_GrabMinSize;
			//j["v_ImGuiStyleVar_GrabRounding"] = v_ImGuiStyleVar_GrabRounding;
			//j["v_ImGuiStyleVar_LayoutAlign"] = v_ImGuiStyleVar_LayoutAlign;
			return j;
		}
		
	};
}