#pragma once
#include "ImGuiElement.h"
#include <string>
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

	virtual void RenderProperties() override
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
		
		
		ImGui::Button(v_label.c_str(), v_size);
		

		if (color_pops)
			ImGui::PopStyleColor(color_pops);
	}
};
