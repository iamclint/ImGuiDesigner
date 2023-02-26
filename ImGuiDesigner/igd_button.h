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
			v_foreground = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_background = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_background_hovered = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_background_active = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
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
			igd::properties->PropertyLabel("Rounding");
			ImGui::SliderFloat("##property_rounding", &this->v_ImGuiStyleVar_FrameRounding, 0, 36);
			igd::properties->PropertyLabel("Disabled Alpha");
			ImGui::SliderFloat("##property_disabled_alpha", &this->v_ImGuiStyleVar_DisabledAlpha, 0, 1);
			igd::properties->PropertyLabel("Frame Border Size");
			ImGui::InputFloat("##property_frame_border_size", (float*)&this->v_ImGuiStyleVar_FrameBorderSize);
			igd::properties->PropertyLabel("Frame Padding");
			ImGui::InputFloat2("##property_frame_padding", (float*)&this->v_ImGuiStyleVar_FramePadding);
			igd::properties->PropertyLabel("Text Align");
			ImGui::InputFloat2("##property_button_align", (float*)&this->v_ImGuiStyleVar_ButtonTextAlign);
		}

		virtual void RenderHead() override
		{
			ImGuiContext& g = *GImGui;
			this->PushStyleColor(ImGuiCol_Text, v_foreground.Value);
			this->PushStyleColor(ImGuiCol_Button, v_background.Value);
			this->PushStyleColor(ImGuiCol_ButtonHovered, v_background_hovered.Value);
			this->PushStyleColor(ImGuiCol_ButtonActive, v_background_active.Value);
			if (this->v_ImGuiStyleVar_FrameRounding>0)
				this->PushStyleVar(ImGuiStyleVar_FrameRounding, this->v_ImGuiStyleVar_FrameRounding);
			if (this->v_ImGuiStyleVar_DisabledAlpha > 0)
				this->PushStyleVar(ImGuiStyleVar_DisabledAlpha, this->v_ImGuiStyleVar_DisabledAlpha);
			if (this->v_ImGuiStyleVar_FramePadding.x > 0 || v_ImGuiStyleVar_FramePadding.y > 0)
				this->PushStyleVar(ImGuiStyleVar_FramePadding, this->v_ImGuiStyleVar_FramePadding);
			if (this->v_ImGuiStyleVar_FrameBorderSize > 0)
				this->PushStyleVar(ImGuiStyleVar_FrameBorderSize, this->v_ImGuiStyleVar_FrameBorderSize);
			if (this->v_ImGuiStyleVar_ButtonTextAlign.x > 0 || v_ImGuiStyleVar_ButtonTextAlign.y > 0)
				this->PushStyleVar(ImGuiStyleVar_ButtonTextAlign, this->v_ImGuiStyleVar_ButtonTextAlign);
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
			//v_id = data["id"];
			v_id = data["id"].get<std::string>() + "##" + RandomID(10);
			v_label = data["label"];
			v_foreground = ImVec4(data["foreground"][0], data["foreground"][1], data["foreground"][2], data["foreground"][3]);
			v_background = ImVec4(data["background"][0], data["background"][1], data["background"][2], data["background"][3]);
			v_background_hovered = ImVec4(data["background_hovered"][0], data["background_hovered"][1], data["background_hovered"][2], data["background_hovered"][3]);
			v_background_active = ImVec4(data["background_active"][0], data["background_active"][1], data["background_active"][2], data["background_active"][3]);
			v_size = ImVec2(data["size"][0], data["size"][1]);
			v_disabled = data["disabled"];
			v_pos = ImVec2(data["pos"][0], data["pos"][1]);
			v_ImGuiStyleVar_FrameRounding = data["v_ImGuiStyleVar_FrameRounding"];
			v_ImGuiStyleVar_DisabledAlpha = data["v_ImGuiStyleVar_DisabledAlpha"];
			v_ImGuiStyleVar_FrameBorderSize = data["v_ImGuiStyleVar_FrameBorderSize"];
			v_ImGuiStyleVar_FramePadding = ImVec2(data["v_ImGuiStyleVar_FramePadding"][0], data["v_ImGuiStyleVar_FramePadding"][1]);
			v_ImGuiStyleVar_ButtonTextAlign = ImVec2(data["v_ImGuiStyleVar_ButtonTextAlign"][0], data["v_ImGuiStyleVar_ButtonTextAlign"][1]);
			
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			j["type"] = "button";
			int pound_pos = v_id.find("#");
			if (pound_pos != std::string::npos)
				j["id"] = v_id.substr(0, pound_pos);
			else
				j["id"] = v_id;
			j["label"] = v_label;
			j["size"] = { v_size.x, v_size.y };
			j["pos"] = { v_pos.x, v_pos.y };
			j["foreground"] = { v_foreground.Value.x, v_foreground.Value.y, v_foreground.Value.z, v_foreground.Value.w};
			j["background"] = { v_background.Value.x, v_background.Value.y, v_background.Value.z, v_background.Value.w };
			j["background_hovered"] = { v_background_hovered.Value.x, v_background_hovered.Value.y, v_background_hovered.Value.z, v_background_hovered.Value.w };
			j["background_active"] = { v_background_active.Value.x, v_background_active.Value.y, v_background_active.Value.z, v_background_active.Value.w };
			j["disabled"] = v_disabled;
			j["v_ImGuiStyleVar_DisabledAlpha"] = v_ImGuiStyleVar_DisabledAlpha;
			j["v_ImGuiStyleVar_FramePadding"] = { v_ImGuiStyleVar_FramePadding.x,v_ImGuiStyleVar_FramePadding.y };
			j["v_ImGuiStyleVar_FrameRounding"] = v_ImGuiStyleVar_FrameRounding;
			j["v_ImGuiStyleVar_FrameBorderSize"] = v_ImGuiStyleVar_FrameBorderSize;
			j["v_ImGuiStyleVar_ButtonTextAlign"] = { v_ImGuiStyleVar_ButtonTextAlign.x,v_ImGuiStyleVar_ButtonTextAlign.y };
			return j;
		}
};
}