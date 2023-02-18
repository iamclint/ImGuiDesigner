#pragma once
#include "ImGuiElement.h"
#include <string>
#include <vector>
class igd_childwindow : ImGuiElement
{
public:
	std::vector<ImGuiElement*> children;
	igd_childwindow() {
		v_flags = ImGuiButtonFlags_None;
		v_size = ImVec2(0, 0);
		v_id = ("new button##" + random_string(10)).c_str();
		v_label = "new button";
		v_foreground = ImColor(255, 255, 255, 255);
		v_background = ImColor(0, 0, 0, 255);
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
		ImGui::BeginChild(v_id.c_str(), v_size, border, v_flags);
		//iterate all children
		ImGui::EndChild();
		if (color_pops)
			ImGui::PopStyleColor(color_pops);
	}

};
