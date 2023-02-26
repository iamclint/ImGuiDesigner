#include "ImGuiElement.h"
#include <iostream>
#include "Properties.h"
#include "imgui_internal.h"
#include <Windows.h>
#include "Workspace.h"
#include "ImGuiDesigner.h"
#include <fstream>
#include <filesystem>
#include <random>
#include "Notifications.h"
#define IMGUI_DEFINE_MATH_OPERATORS
//get mouse location delta of item

void GetAllChildren(ImGuiElement* parent,nlohmann::json& pjson)
{
	ImGuiContext& g = *GImGui;
	for (int i =0;auto& e : parent->children)
	{
		if (e->delete_me)
			continue;
		try
		{
			pjson["children"].push_back(e->GetJson());
		}
		catch (nlohmann::json::exception& e)
		{
			std::cout << "exception: " << e.what() << std::endl;
		}
		if (e->v_can_have_children)
		{
			GetAllChildren(e, pjson["children"].back());
		}
		i++;
	}
}

void ImGuiElement::PushStyleColor(ImGuiCol idx, const ImVec4& col)
{
	igd::active_workspace->code << "ImGui::PushStyleColor(" << idx << ", ImVec4(" << col.x << ", " << col.y << ", " << col.z << ", " << col.w << "));" << std::endl;
	ImGui::PushStyleColor(idx, col);
	color_pops++;
}
void ImGuiElement::PushStyleVar(ImGuiStyleVar idx, float val)
{
	igd::active_workspace->code << "ImGui::PushStyleVar(" << idx << ", " << val << ");" << std::endl;
	ImGui::PushStyleVar(idx, val);
	style_pops++;
}
void ImGuiElement::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val)
{
	igd::active_workspace->code << "ImGui::PushStyleVar(" << idx << ", ImVec2(" << val.x << ", " << val.y << "));" << std::endl;
	ImGui::PushStyleVar(idx, val);
	style_pops++;
}

void ImGuiElement::SaveAsWidget(std::string name)
{
	if (children.size() == 0)
		return;
	//check if folder exists
	if (!std::filesystem::exists("widgets"))
		std::filesystem::create_directory("widgets");
	
	int find_pound = name.find("#");
	if (find_pound != std::string::npos)
		name = name.substr(0, find_pound);
	//check if file exists
	if (std::filesystem::exists("widgets/" + name + ".wgd"))
	{
		//if it does, do something
		igd::notifications->GenericNotification("File already exists", "Please choose a new id for this widget", "", "Ok", []() { std::cout << "Yay" << std::endl; });
		return;
	}

	std::ofstream file;
	file.open("widgets/" + name + ".wgd");
	nlohmann::json main_obj;
	main_obj["child_window"] = this->GetJson();
	for (auto& e : children)
	{
		if (e->delete_me)
			continue;
		main_obj["child_window"]["children"].push_back(e->GetJson());
		if (e->v_can_have_children)
			GetAllChildren(e, main_obj["child_window"]["children"].back());
	}
	std::cout << main_obj.dump() << std::endl;
	file << main_obj.dump() << std::endl;
	file.close();
	std::cout << "Saved to widgets/widget.igd" << std::endl;
}

void ImGuiElement::Undo()
{
	UndoLocal();
	did_move = false;
	did_resize = false;

}
void ImGuiElement::Redo()
{
	RedoLocal();
	did_move = false;
	did_resize = false;

}

void ImGuiElement::PushUndo()
{
	//add an undo layer to the undo stack for the workspace itself
	igd::active_workspace->PushUndo(this);
	PushUndoLocal();
}

ImVec2 get_mouse_location()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();
	ImVec2 window_pos = ImGui::GetWindowPos();
	return ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
}

bool ImGuiElement::Drag()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImVec2 Item_Location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && resize==resize_direction::none)
	{
		is_dragging = true;
		ImVec2 mouse_location = get_mouse_location();
		current_drag_delta = { mouse_location.x - Item_Location.x - ImGui::GetScrollX(),mouse_location.y - Item_Location.y - ImGui::GetScrollY() };
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && is_dragging)
	{
		did_move = true;
		g.MouseCursor = ImGuiMouseCursor_ResizeAll;
		ImVec2 mouse_location = get_mouse_location();
		v_pos = { mouse_location.x - current_drag_delta.x,mouse_location.y - current_drag_delta.y };
	}
	return is_dragging;
}

void ImGuiElement::KeyMove()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x + ImGui::GetScrollX(), g.LastItemData.Rect.Min.y - window->Pos.y + ImGui::GetScrollY());
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
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !ImGui::IsAnyItemActive())
	{
		igd::notifications->Confirmation("Delete", "Are you sure you wish to delete " + this->v_id, "", [this](bool conf) {
			if (!conf)
				return;
			if (this->children.size() > 0)
			{
				for (auto& child : this->children)
					child->Delete();
			}
			this->Delete();
		});
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
	{
		ImGui::GetIO().KeysDown[ImGuiKey_Escape] = false;
		igd::properties->active_element = nullptr;
	}
}


void ImGuiElement::Delete()
{
	if (igd::properties->active_element)
	{
		igd::active_workspace->undo_stack.push_back(this);
		igd::properties->active_element->delete_me = true;
		if (igd::properties->copied_element == igd::properties->active_element)
			igd::properties->copied_element = nullptr;
		igd::properties->active_element = nullptr;
	}
}



bool ImGuiElement::Resize()
{
	float delta_offset = 10.0f;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	ImVec2 current_size = ImVec2(g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.y - g.LastItemData.Rect.Min.y);
	ImVec2 mouse_delta_br = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_bl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_tr = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };
	ImVec2 mouse_delta_tl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };
	bool is_mouse_hovering_br = fabs(mouse_delta_br.x) < delta_offset && fabs(mouse_delta_br.y) < delta_offset;
	bool is_mouse_hovering_r = fabs(mouse_delta_br.x) < delta_offset && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_tr = fabs(mouse_delta_tr.x) < delta_offset && fabs(mouse_delta_tr.y) < delta_offset;
	bool is_mouse_hovering_l = fabs(mouse_delta_bl.x) < delta_offset && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_b = fabs(mouse_delta_bl.y) < delta_offset && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_t = fabs(mouse_delta_tr.y) < delta_offset && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_bl = fabs(mouse_delta_bl.x) < delta_offset && fabs(mouse_delta_bl.y) < delta_offset;
	bool is_mouse_hovering_tl = fabs(mouse_delta_tl.x) < delta_offset && fabs(mouse_delta_tl.y) < delta_offset;
	
	if (resize == resize_direction::none)
	{
		if (is_mouse_hovering_br)
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
		else if (is_mouse_hovering_r)
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
		else if (is_mouse_hovering_tr)
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
		else if (is_mouse_hovering_l)
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
		else if (is_mouse_hovering_b)
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
		else if (is_mouse_hovering_t)
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
		else if (is_mouse_hovering_bl)
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
		else if (is_mouse_hovering_tl)
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !is_dragging && resize == resize_direction::none)
	{
		if (is_mouse_hovering_br)
			resize = resize_direction::bottom_right;
		else if (is_mouse_hovering_r)
			resize = resize_direction::right;
		else if (is_mouse_hovering_tr)
			resize = resize_direction::top_right;
		else if (is_mouse_hovering_l)
			resize = resize_direction::left;
		else if (is_mouse_hovering_b)
			resize = resize_direction::bottom;
		else if (is_mouse_hovering_t)
			resize = resize_direction::top;
		else if (is_mouse_hovering_bl)
			resize = resize_direction::bottom_left;
		else if (is_mouse_hovering_tl)
			resize = resize_direction::top_left;
		
		last_size = current_size;
		last_position = item_location;
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && resize!=resize_direction::none)
	{
		mouse_drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
		did_resize = true;
		switch (resize)
		{
			case resize_direction::top_right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
				v_size = { last_size.x + mouse_drag_delta.x,last_size.y - mouse_drag_delta.y };
				v_pos.y = last_position.y + mouse_drag_delta.y;
				break;
			}
			case resize_direction::top_left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
				v_size = { last_size.x - mouse_drag_delta.x,last_size.y - mouse_drag_delta.y };
				v_pos.y = last_position.y + mouse_drag_delta.y;
				v_pos.x = last_position.x + mouse_drag_delta.x;
				break;
			}
			case resize_direction::bottom_left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
				v_size = { last_size.x - mouse_drag_delta.x,last_size.y + mouse_drag_delta.y };
				//v_pos.y = last_position.y + mouse_drag_delta.y;
				v_pos.x = last_position.x + mouse_drag_delta.x;
				break;
			}
			case resize_direction::left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				v_size = { last_size.x - mouse_drag_delta.x, last_size.y };
				v_pos.x = last_position.x + mouse_drag_delta.x;
				break;
			}
			case resize_direction::bottom_right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
				v_size = { last_size.x + mouse_drag_delta.x,last_size.y + mouse_drag_delta.y };
				break;
			}
			case resize_direction::right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				v_size = { last_size.x + mouse_drag_delta.x,last_size.y };
				break;
			}
			case resize_direction::top:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNS;
				v_size = { last_size.x ,last_size.y - mouse_drag_delta.y };
				v_pos.y = last_position.y + mouse_drag_delta.y;
				break;
			}
			case resize_direction::bottom:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNS;
				v_size = { last_size.x,last_size.y + mouse_drag_delta.y};
				break;
			}
		}
	}
	return resize != resize_direction::none;
}

void ImGuiElement::Select()
{
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !is_dragging && resize == resize_direction::none)
	{
		igd::properties->active_element = this;
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
	ImGuiContext& g = *GImGui;
	if (v_pos.x != 0 || v_pos.y != 0)
		ImGui::SetCursorPos(v_pos);
	
	color_pops = 0;
	style_pops = 0;
	if (v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
	{
		igd::active_workspace->code << "ImGui::BeginDisabled();" << std::endl;
		ImGui::BeginDisabled();
	}
	
	this->RenderHead();
	if (this->children.size() > 0)
	{
		for (auto& child : this->children)
		{
			if (child->delete_me)
				continue;
			child->Render();
		}

		for (auto it = this->children.begin(); it != this->children.end();)
		{
			if (!(*it)->v_parent)
			{
				igd::active_workspace->elements.push_back(*it);
				it = this->children.erase(it);
			}
			else if ((*it)->v_parent!=this)
			{
				(*it)->v_parent->children.push_back(*it);
				it = this->children.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	
	this->RenderInternal();
	this->RenderFoot();
	
	if (v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0)
	{
		ImGui::EndDisabled();
		igd::active_workspace->code << "ImGui::EndDisabled();" << std::endl;
	}

	if (style_pops > 0)
	{
		ImGui::PopStyleVar(style_pops);
		igd::active_workspace->code << "ImGui::PopStyleVar(" << style_pops << ");" << std::endl;
	}
	if (color_pops > 0)
	{
		ImGui::PopStyleColor(color_pops);
		igd::active_workspace->code << "ImGui::PopStyleColor(" << color_pops << ");" << std::endl;
	}
	//reset imgui cursorpos so you don't interrupt the flow of other elements when you drag this one
	if (v_pos.x != 0 || v_pos.y != 0)
		ImGui::SetCursorPos(last_known_cursor);
	else
		last_known_cursor = ImGui::GetCursorPos();
	
	if (igd::properties->active_element == this)
	{
		Resize();
		Drag();
		KeyMove();
		DrawSelection();
		KeyBinds();
	}
	if (g.MouseCursor == ImGuiMouseCursor_Hand || g.MouseCursor == ImGuiMouseCursor_Arrow)
		Select();
	
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		if (did_resize)
		{
			PushUndo();
			did_resize = false;
		}
		resize = resize_direction::none;
		if (did_move)
		{
			PushUndo();
			did_move = false;
		}
		is_dragging = false;
	}
}

//random number generator
int GetRandomInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(min, max);
	return dist(mt);
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
		return charset[GetRandomInt(0,max_index)];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}