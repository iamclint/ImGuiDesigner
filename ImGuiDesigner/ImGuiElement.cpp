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
static inline std::unordered_map<ImGuiElement*, std::vector<ImGuiElement>> undo_stack;
static inline std::unordered_map<ImGuiElement*, std::vector<ImGuiElement>> redo_stack;
void ImGuiElement::UndoLocal() {
	if (undo_stack[this].size() > 1)
	{
		redo_stack[this].push_back(*this);
		if (undo_stack[this].size() > 1)
			undo_stack[this].pop_back();

		*this = undo_stack[this].back();
	}
}
void ImGuiElement::RedoLocal()
{
	if (redo_stack[this].size() > 0)
	{
		*this = redo_stack[this].back();
		PushUndo();
		redo_stack[this].pop_back();
	}
}

void ImGuiElement::PushUndoLocal()
{
	//keep an undo stack locally for this type
	undo_stack[this].push_back(*this);
}
ImGuiElement::ImGuiElement()
	: v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID(10)), v_label("new element"),
	v_foreground(ImColor(0, 0, 0, 0)), v_background(ImColor(0, 0, 0, 0)), v_parent(nullptr), v_border(0),
	v_pos(ImVec2(0, 0)), is_dragging(false), resize(resize_direction::none), current_drag_delta(0, 0), last_size(0, 0),
	delete_me(false), v_can_have_children(false), change_parent(nullptr), did_resize(false), did_move(false),
	v_disabled(false), v_property_flags(property_flags::None), color_pops(0), style_pops(0), v_ImGuiStyleVar_Alpha(0),
	v_ImGuiStyleVar_DisabledAlpha(0), v_ImGuiStyleVar_WindowPadding(0, 0), v_ImGuiStyleVar_WindowRounding(0),
	v_ImGuiStyleVar_WindowBorderSize(0), v_ImGuiStyleVar_WindowMinSize(0, 0), v_ImGuiStyleVar_WindowTitleAlign(0, 0),
	v_ImGuiStyleVar_ChildRounding(0), v_ImGuiStyleVar_ChildBorderSize(0), v_ImGuiStyleVar_PopupRounding(0),
	v_ImGuiStyleVar_PopupBorderSize(0), v_ImGuiStyleVar_FramePadding(0, 0), v_ImGuiStyleVar_FrameRounding(0),
	v_ImGuiStyleVar_FrameBorderSize(0), v_ImGuiStyleVar_ItemSpacing(0, 0), v_ImGuiStyleVar_ItemInnerSpacing(0, 0),
	v_ImGuiStyleVar_IndentSpacing(0), v_ImGuiStyleVar_CellPadding(0, 0), v_ImGuiStyleVar_ScrollbarSize(0),
	v_ImGuiStyleVar_ScrollbarRounding(0), v_ImGuiStyleVar_GrabMinSize(0), v_ImGuiStyleVar_GrabRounding(0),
	v_ImGuiStyleVar_TabRounding(0), v_ImGuiStyleVar_ButtonTextAlign(0, 0), v_ImGuiStyleVar_SelectableTextAlign(0, 0),
	v_ImGuiStyleVar_LayoutAlign(0)
{
	v_property_flags = property_flags::color_foreground | property_flags::color_background | property_flags::disabled;
}
void ImGuiElement::RenderPropertiesInternal()
{
	igd::properties->PropertyLabel("Child Rounding");
	ImGui::SliderFloat("##property_rounding", &this->v_ImGuiStyleVar_ChildRounding, 0, 36);
	igd::properties->PropertyLabel("Frame Rounding");
	ImGui::SliderFloat("##property_frame_rounding", &this->v_ImGuiStyleVar_FrameRounding, 0, 36);
	igd::properties->PropertyLabel("Border Size");
	ImGui::SliderFloat("##property_border_size", &this->v_ImGuiStyleVar_ChildBorderSize, 0, 36);
	igd::properties->PropertyLabel("Indent Spacing");
	ImGui::SliderFloat("##property_indent_spacing", &this->v_ImGuiStyleVar_IndentSpacing, 0, 36);
	igd::properties->PropertyLabel("Scrollbar Size");
	ImGui::SliderFloat("##property_scrollbar_size", &this->v_ImGuiStyleVar_ScrollbarSize, 0, 36);
	igd::properties->PropertyLabel("Grabber Min");
	ImGui::SliderFloat("##property_Grabber_Min", &this->v_ImGuiStyleVar_GrabMinSize, 0, 36);
	igd::properties->PropertyLabel("Grabber Rounding");
	ImGui::SliderFloat("##property_Grabber_Rounding", &this->v_ImGuiStyleVar_GrabRounding, 0, 36);
	igd::properties->PropertyLabel("Layout Align");
	ImGui::InputFloat2("##property_Layout_Align", (float*)&this->v_ImGuiStyleVar_LayoutAlign);
	igd::properties->PropertyLabel("Frame Padding");
	ImGui::InputFloat2("##property_frame_padding", (float*)&this->v_ImGuiStyleVar_FramePadding);
	igd::properties->PropertyLabel("Item Spacing");
	ImGui::InputFloat2("##property_item_spacing", (float*)&this->v_ImGuiStyleVar_ItemSpacing);
	igd::properties->PropertyLabel("Item Inner Spacing");
	ImGui::InputFloat2("##property_item_inner_spacing", (float*)&this->v_ImGuiStyleVar_ItemInnerSpacing);
}
void ImGuiElement::FromJSON(nlohmann::json data)
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
	v_ImGuiStyleVar_FrameRounding = data["v_ImGuiStyleVar_FrameRounding"];
	v_ImGuiStyleVar_ChildBorderSize = data["v_ImGuiStyleVar_ChildBorderSize"];
	v_ImGuiStyleVar_FramePadding = ImVec2(data["v_ImGuiStyleVar_FramePadding"][0], data["v_ImGuiStyleVar_FramePadding"][1]);
	v_ImGuiStyleVar_ItemSpacing = ImVec2(data["v_ImGuiStyleVar_ItemSpacing"][0], data["v_ImGuiStyleVar_ItemSpacing"][1]);
	v_ImGuiStyleVar_ItemInnerSpacing = ImVec2(data["v_ImGuiStyleVar_ItemInnerSpacing"][0], data["v_ImGuiStyleVar_ItemInnerSpacing"][1]);
	v_ImGuiStyleVar_IndentSpacing = data["v_ImGuiStyleVar_IndentSpacing"];
	v_ImGuiStyleVar_ScrollbarSize = data["v_ImGuiStyleVar_ScrollbarSize"];
	v_ImGuiStyleVar_GrabMinSize = data["v_ImGuiStyleVar_GrabMinSize"];
	v_ImGuiStyleVar_GrabRounding = data["v_ImGuiStyleVar_GrabRounding"];
	v_ImGuiStyleVar_LayoutAlign = data["v_ImGuiStyleVar_LayoutAlign"];
}
nlohmann::json ImGuiElement::GetJson()
{
	nlohmann::json j;
	j["type"] = "basic";
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
	j["v_ImGuiStyleVar_GrabMinSize"] = v_ImGuiStyleVar_GrabMinSize;
	j["v_ImGuiStyleVar_GrabRounding"] = v_ImGuiStyleVar_GrabRounding;
	j["v_ImGuiStyleVar_LayoutAlign"] = v_ImGuiStyleVar_LayoutAlign;
}
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
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !ImGui::IsAnyItemActive() && !igd::notifications->IsShowing())
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
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !ImGui::IsAnyItemActive() && !igd::notifications->IsShowing())
		{
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

void ImGuiElement::PopColorAndStyles()
{
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
	color_pops = 0;
	style_pops = 0;
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
	PopColorAndStyles();

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