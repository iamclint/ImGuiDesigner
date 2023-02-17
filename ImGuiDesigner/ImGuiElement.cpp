#include "ImGuiElement.h"
#include <iostream>
#include "Properties.h"
#define IMGUI_DEFINE_MATH_OPERATORS
//get mouse location delta of item


ImVec2 get_mouse_location()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();
	ImVec2 window_pos = ImGui::GetWindowPos();
	return ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
}

void ImGuiElement::Drag(ImVec2 Item_Location)
{
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		igd::properties->active_element = this;
		is_dragging = true;
		std::cout << "start drag" << std::endl;
		ImVec2 mouse_location = get_mouse_location();
		current_drag_delta = { mouse_location.x - Item_Location.x,mouse_location.y - Item_Location.y };
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && is_dragging && igd::properties->active_element==this)
	{
		ImVec2 mouse_location = get_mouse_location();
		v_pos = { mouse_location.x - current_drag_delta.x,mouse_location.y - current_drag_delta.y };
	}

	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		is_dragging = false;
}

std::string ImGuiElement::random_string(size_t length)
{
		auto randchar = []() -> char
		{
			const char charset[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[rand() % max_index];
		};
		std::string str(length, 0);
		std::generate_n(str.begin(), length, randchar);
		return str;
}