#pragma once
#include "ImGuiElement.h"
#include <string>
class igd_button : ImGuiElement
{
public:
	
	igd_button() {
		v_flags = ImGuiButtonFlags_None;
		v_size = ImVec2(0, 0);
		v_id = ("new button##" + random_string(10)).c_str();
		v_label = "new button";
		v_foreground = ImColor(0, 0, 0, 0);
		v_background = ImColor(0, 0, 0, 0);
	}

	
	virtual void Render() override
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
		
		if (v_pos.x != 0 || v_pos.y != 0)
		{
			ImGui::SetCursorPos(v_pos);
		}
		ImVec2 Item_Location = ImGui::GetCursorPos(); //used for generating a delta on drag;
		ImGui::Button(v_label.c_str(), v_size);
		if (v_pos.x != 0 || v_pos.y != 0)
			ImGui::SetCursorPos(last_known_cursor);
		else
			last_known_cursor = ImGui::GetCursorPos();
		Drag(Item_Location);
		
		

		
		if (color_pops)
			ImGui::PopStyleColor(color_pops);
	}
	
};
