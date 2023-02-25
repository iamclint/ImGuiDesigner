#pragma once
#include "ImGuiElement.h"
#include <string>
#include <vector>
#include "Workspace.h"
#include "../json/single_include/nlohmann/json.hpp"
namespace igd
{


	class ChildWindow : ImGuiElement
	{
	public:
		int color_pops;
		static inline std::unordered_map<ChildWindow*, std::vector<ChildWindow>> undo_stack;
		static inline std::unordered_map<ChildWindow*, std::vector<ChildWindow>> redo_stack;
		ChildWindow() {
			color_pops = 0;
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

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{

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

		virtual void RenderHead() override
		{
			ImGuiContext& g = *GImGui;

			if (v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
				ImGui::BeginDisabled();
			
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
			ImGuiContext& g = *GImGui;
			if (v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0)
				ImGui::EndDisabled();
			
			if (color_pops)
				ImGui::PopStyleColor(color_pops);
		}
		virtual void FromJSON(nlohmann::json data) override
		{
			v_flags = data["flags"];
			v_size = ImVec2(data["size"][0], data["size"][1]);
		//	v_id = data["id"].get<std::string>().c_str();
			v_label = data["label"].get<std::string>().c_str();
			v_foreground = ImVec4(data["foreground"][0], data["foreground"][1], data["foreground"][2], data["foreground"][3]);
			v_background = ImVec4(data["background"][0], data["background"][1], data["background"][2], data["background"][3]);
			v_border = data["border"];
			v_property_flags = data["property_flags"];
			v_disabled = data["disabled"];
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			j["type"] = "child window";
			//j["id"] = v_id;
			j["label"] = v_label;
			j["size"] = { v_size.x, v_size.y };
			j["flags"] = v_flags;
			j["foreground"] = { v_foreground.Value.x, v_foreground.Value.y, v_foreground.Value.z, v_foreground.Value.w };
			j["background"] = { v_background.Value.x, v_background.Value.y, v_background.Value.z, v_background.Value.w };
			j["border"] = v_border;
			j["disabled"] = v_disabled;
			j["property_flags"] = v_property_flags;
			return j;
		}
		
	};
}