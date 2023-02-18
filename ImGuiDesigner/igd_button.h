#pragma once
#include "ImGuiElement.h"
#include <string>
#include "misc/cpp/imgui_stdlib.h"
class igd_button : ImGuiElement
{
public:
	
	igd_button() {
		v_flags = ImGuiButtonFlags_None;
		v_size = ImVec2(0, 0);
		v_id = ("new button##" + RandomID(10)).c_str();
		v_label = "new button";
		v_foreground = ImColor(0, 0, 0, 0);
		v_background = ImColor(0, 0, 0, 0);
	}

	virtual void Clone() override
	{
		igd::work->elements_buffer.push_back((ImGuiElement*)(new igd_button()));
		igd::work->elements_buffer.back()->v_background = this->v_background;
		igd::work->elements_buffer.back()->v_foreground = this->v_foreground;
		igd::work->elements_buffer.back()->v_flags = this->v_flags;
		igd::work->elements_buffer.back()->v_label = this->v_label;
		igd::work->elements_buffer.back()->v_size = this->v_size;
	}
	
	virtual void RenderPropertiesInternal() override
	{
		
	}
	
	virtual void RenderInternal() override
	{
		int color_pops = 0;
		if (v_foreground.Value.w != 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, v_foreground.Value);
			color_pops++;
		}
		if (v_background.Value.w != 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, v_background.Value);
			color_pops++;
		}
		
		ImGui::Button((v_label+"##"+v_id).c_str(), v_size);
		

		if (color_pops)
			ImGui::PopStyleColor(color_pops);
	}
};
