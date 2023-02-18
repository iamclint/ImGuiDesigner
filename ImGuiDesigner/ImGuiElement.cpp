#include "ImGuiElement.h"
#include <iostream>
#include "Properties.h"
#include "imgui_internal.h"
#include <Windows.h>
#include "Workspace.h"
#define IMGUI_DEFINE_MATH_OPERATORS
//get mouse location delta of item


ImVec2 get_mouse_location()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();
	ImVec2 window_pos = ImGui::GetWindowPos();
	return ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
}

void ImGuiElement::Drag()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImVec2 Item_Location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !is_resizing)
	{
		is_dragging = true;
		ImVec2 mouse_location = get_mouse_location();
		current_drag_delta = { mouse_location.x - Item_Location.x,mouse_location.y - Item_Location.y };
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && is_dragging)
	{
		ImVec2 mouse_location = get_mouse_location();
		v_pos = { mouse_location.x - current_drag_delta.x,mouse_location.y - current_drag_delta.y };
	}
}

void ImGuiElement::KeyMove()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	float delta_dist = 1.f;
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
	{
		v_pos.y = item_location.y - delta_dist;
		v_pos.x = item_location.x;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
	{
		v_pos.y = item_location.y + delta_dist;
		v_pos.x = item_location.x;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
	{
		v_pos.x = item_location.x - delta_dist;
		v_pos.y = item_location.y;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
	{
		v_pos.x = item_location.x + delta_dist;
		v_pos.y = item_location.y;
	}
}


void ImGuiElement::KeyBinds()
{
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && ImGui::GetIO().KeyCtrl)
	{
		igd::properties->copied_element = this;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && ImGui::GetIO().KeyCtrl)
	{
		if (igd::properties->copied_element)
			igd::properties->copied_element->Clone();
		
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
	{
		this->Delete();
	}
}

void ImGuiElement::Delete()
{
	igd::properties->active_element->delete_me = true;
	if (igd::properties->copied_element == igd::properties->active_element)
		igd::properties->copied_element = nullptr;
	igd::properties->active_element = nullptr;
}

void ImGuiElement::Resize()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	ImVec2 current_size = ImVec2(g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.y - g.LastItemData.Rect.Min.y);
	ImVec2 mouse_delta_br = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	bool is_mouse_hovering_br = mouse_delta_br.x < 8 && mouse_delta_br.y >= 0 && mouse_delta_br.y < 8 && mouse_delta_br.y >= 0;
	if (is_mouse_hovering_br)
		g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;

	if (is_mouse_hovering_br && ImGui::IsMouseClicked(0) && !is_dragging)
	{
		
		is_resizing = true;
		last_size = current_size;
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && is_resizing)
	{
		ImVec2 mouse_drag_delta = ImGui::GetMouseDragDelta(0);
		v_size = { last_size.x + mouse_drag_delta.x,last_size.y + mouse_drag_delta.y };
	}
}

void ImGuiElement::Select()
{
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !is_dragging && !is_resizing)
	{
		igd::properties->active_element = this;
	}
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		is_resizing = false;
		is_dragging = false;
	}
}

void ImGuiElement::DrawSelection()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImRect item_location = ImRect({ g.LastItemData.Rect.Min.x - 4,g.LastItemData.Rect.Min.y - 4 }, { g.LastItemData.Rect.Max.x + 4,g.LastItemData.Rect.Max.y + 4 });
	ImGui::GetForegroundDrawList()->AddRect(item_location.Min, item_location.Max, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, 0, 2.0f);
}



void ImGuiElement::Render()
{
	if (v_pos.x != 0 || v_pos.y != 0)
		ImGui::SetCursorPos(v_pos);
	
	this->RenderInternal();
	
	//reset imgui cursorpos so you don't interrupt the flow of other elements when you drag this one
	if (v_pos.x != 0 || v_pos.y != 0)
		ImGui::SetCursorPos(last_known_cursor);
	else
		last_known_cursor = ImGui::GetCursorPos();
	
	Select();
	if (igd::properties->active_element == this)
	{
		Resize();
		Drag();
		KeyMove();
		DrawSelection();
		KeyBinds();
	}
}

std::string ImGuiElement::RandomID(size_t length)
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