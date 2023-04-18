#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962
#endif
void ImGuiElement::HandleHover()
{
	if (is_child_hovered)
		return;
	if (this->v_type_id == (int)element_type::window && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
	{
		igd::active_workspace->hovered_element = this;
	}
	else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
	{
		igd::active_workspace->hovered_element = this;
	}
}

//called from workspace once per frame on the hovered element
void ImGuiElement::HandleDrop()
{
	if (!this->v_can_have_children)
		return;

	//don't do anything if this is in the selected list (can't drop this to this)
	for (auto& e : igd::active_workspace->selected_elements)
	{
		if (e == this)
			return;
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && this->drop_new_parent)
	{
		igd::active_workspace->ResetSelectTimeout();
		for (auto& e : igd::active_workspace->selected_elements)
		{
			if (this != e && e->v_parent != this && e != igd::active_workspace->basic_workspace_element)
			{

				e->v_parent = this;
				ImVec2 final_pos = e->v_pos_dragging.value - e->v_parent->item_rect.Min + e->v_parent->v_scroll_position;
				if (e->v_pos.type == Vec2Type::Absolute)
					e->v_pos.value = final_pos;
				else
				{
					e->v_pos.value.x = (final_pos.x / e->v_parent->ContentRegionAvailSelf.x) * 100;
					e->v_pos.value.y = (final_pos.y / e->v_parent->ContentRegionAvailSelf.y) * 100;
				}

			}
		}
		this->drop_new_parent = false;
	}

	if (!igd::active_workspace->is_dragging)
	{
		this->drop_new_parent = false;
		return;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && igd::active_workspace->selected_elements.size() > 0 && igd::active_workspace->selected_elements[0]->v_parent!=this)
	{
		ImRect item_location = ImRect({ this->item_rect.Min.x - 4,this->item_rect.Min.y - 4 }, { this->item_rect.Max.x + 4,this->item_rect.Max.y + 4 });
		ImGui::GetForegroundDrawList()->AddRectFilled(item_location.Min, item_location.Max, ImColor(34, 205, 141, 26), 1.f, 0);
		this->drop_new_parent = true;
	}
}


bool ImGuiElement::Drag()
{
	ImGuiContext& g = *GImGui;

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(0) && resize_direction == ResizeDirection::none && !igd::active_workspace->is_dragging)
	{
		for (auto& e : igd::active_workspace->selected_elements)
		{
			e->v_pos_dragging.value = { e->item_rect.Min.x, e->item_rect.Min.y };
		}
		igd::active_workspace->is_dragging = true;
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && igd::active_workspace->is_dragging)
	{
		igd::active_workspace->ResetSelectTimeout();
		igd::active_workspace->is_dragging = false;
	}

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && igd::active_workspace->is_dragging)
	{
		did_move = true;
		g.MouseCursor = ImGuiMouseCursor_ResizeAll;
		for (auto& e : igd::active_workspace->selected_elements)
		{
			e->ApplyDeltaPosDrag(ImGui::GetMouseDragDelta());
		}
		ImGui::ResetMouseDragDelta();
	}



	return igd::active_workspace->is_dragging;
}

void ImGuiElement::KeyMove()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x + ImGui::GetScrollX(), g.LastItemData.Rect.Min.y - window->Pos.y + ImGui::GetScrollY());
	float delta_dist = 1.f;
	if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
	{
		v_pos.value.y = item_location.y - delta_dist;
		v_pos.value.x = item_location.x;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
	{
		v_pos.value.y = item_location.y + delta_dist;
		v_pos.value.x = item_location.x;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
	{
		v_pos.value.x = item_location.x - delta_dist;
		v_pos.value.y = item_location.y;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
	{
		v_pos.value.x = item_location.x + delta_dist;
		v_pos.value.y = item_location.y;
	}
}
enum class RectSide {
	Top,
	Right,
	Bottom,
	Left
};
RectSide getNearestSide(const ImVec2& center, const ImVec2& point) {
	// Calculate the distance between the point and the center of the rectangle.
	const float dx = point.x - center.x;
	const float dy = point.y - center.y;
	const float distanceSquared = dx * dx + dy * dy;

	// Calculate the angle between the x-axis and the vector from the center to the point.
	const float angle = atan2(dy, dx);

	// Determine which side the point is closest to based on the angle.
	if (angle > -M_PI_4 && angle <= M_PI_4) {
		return RectSide::Right;
	}
	else if (angle > M_PI_4 && angle <= 3 * M_PI_4) {
		return RectSide::Bottom;
	}
	else if (angle > -3 * M_PI_4 && angle <= -M_PI_4) {
		return RectSide::Top;
	}
	else {
		return RectSide::Left;
	}
}
void ImGuiElement::RenderDrag()
{

		ImGuiContext& g = *GImGui;
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoFocusOnAppearing;
		for (auto& e : igd::active_workspace->selected_elements)
		{
		

			if (e->delete_me)
				continue;
			ImGui::SetNextWindowPos(ImVec2(e->v_pos_dragging.value.x, e->v_pos_dragging.value.y), ImGuiCond_Always, ImVec2(0, 0));
			ImGui::SetNextWindowBgAlpha(0);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);



			if (ImGui::Begin(("Drag##Drag_" + e->v_id).c_str(), NULL, flags))
			{
				float side_indicator_size = 2;
				float line_offset = 5;
				ImGuiElement* nearest = igd::GetNearestElement(e);
				if (nearest && igd::GetDistance(e->GetPos(), nearest->GetPos())<250)
				{
					RectSide side = getNearestSide(nearest->GetPos() + ImVec2(nearest->GetRawSize() / 2), e->GetPos() + ImVec2(e->GetRawSize() / 2));// , nearest->item_rect.Max.x - nearest->item_rect.Min.x, nearest->item_rect.Max.y - nearest->item_rect.Min.y);

					switch (side)
					{
					case RectSide::Bottom:
					{
						ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x,nearest->item_rect.Max.y }, { nearest->item_rect.Max.x,nearest->item_rect.Max.y + side_indicator_size }, ImColor(255, 0, 255, 150), 1.f, 0);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Min.x,nearest->item_rect.Max.y }, { nearest->item_rect.Min.x,e->GetPos().y + e->GetRawSize().y + line_offset }, ImColor(255, 255, 255, 60), 1.f);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Max.x,nearest->item_rect.Max.y }, { nearest->item_rect.Max.x,e->GetPos().y + e->GetRawSize().y + line_offset }, ImColor(255, 255, 255, 60), 1.f);
						break;
					}
					case RectSide::Top:
					{
						ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x,nearest->item_rect.Min.y - side_indicator_size }, { nearest->item_rect.Max.x,nearest->item_rect.Min.y }, ImColor(255, 0, 255, 150), 1.f, 0);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Min.x,nearest->item_rect.Min.y }, { nearest->item_rect.Min.x,e->GetPos().y - line_offset }, ImColor(255, 255, 255, 60), 1.f);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Max.x,nearest->item_rect.Min.y }, { nearest->item_rect.Max.x,e->GetPos().y - line_offset }, ImColor(255, 255, 255, 60), 1.f);
						break;
					}
					case RectSide::Left:
					{
						ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x - side_indicator_size,nearest->item_rect.Min.y }, { nearest->item_rect.Min.x,nearest->item_rect.Max.y }, ImColor(255, 0, 255, 150), 1.f, 0);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Min.x,nearest->item_rect.Min.y }, { e->GetPos().x - line_offset,nearest->item_rect.Min.y }, ImColor(255, 255, 255, 60), 1.f);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Min.x,nearest->item_rect.Max.y }, { e->GetPos().x - line_offset,nearest->item_rect.Max.y }, ImColor(255, 255, 255, 60), 1.f);
						break;
					}
					case RectSide::Right:
					{
						ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Max.x,nearest->item_rect.Min.y }, { nearest->item_rect.Max.x + side_indicator_size,nearest->item_rect.Max.y }, ImColor(255, 0, 255, 150), 1.f, 0);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Max.x,nearest->item_rect.Min.y }, { e->GetPos().x + e->GetRawSize().x + line_offset,nearest->item_rect.Min.y }, ImColor(255, 255, 255, 60), 1.f);
						ImGui::GetForegroundDrawList()->AddLine({ nearest->item_rect.Max.x,nearest->item_rect.Max.y }, { e->GetPos().x + e->GetRawSize().x + line_offset,nearest->item_rect.Max.y }, ImColor(255, 255, 255, 60), 1.f);
						break;
					}
					}





					//ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x,nearest->item_rect.Max.y }, { nearest->item_rect.Max.x,nearest->item_rect.Max.y + 5 }, ImColor(255, 0, 255, 150), 1.f, 0);
					std::stringstream ss;
					ss << "Dist: " << igd::GetDistance(e->GetPos(), nearest->GetPos());
					//ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x,nearest->item_rect.Min.y - 5 }, { nearest->item_rect.Max.x,nearest->item_rect.Min.y }, ImColor(255, 0, 255, 150), 1.f, 0);
					ImGui::GetForegroundDrawList()->AddRectFilled(nearest->item_rect.Min, nearest->item_rect.Max, ImColor(0, 0, 0, 255), 1.f, 0);
					ImGui::GetForegroundDrawList()->AddCircleFilled(nearest->GetPos() + ImVec2(nearest->GetSize() / 2), 5, ImColor(255, 0, 255, 150), 10);
					ImGui::GetForegroundDrawList()->AddText(nearest->item_rect.Min, ImColor(0.0f, 1.0f, 0.0f, 1.0f), ss.str().c_str());
				}

				bool need_disable_pop = false;
				color_pops = 0;
				style_pops = 0;
				if (e->v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
				{
					if ((g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
					{
						need_disable_pop = true;
						ImGui::BeginDisabled();
					}
				}
				if (!e->v_inherit_all_colors)
				{
					for (auto& c : e->v_colors)
					{
						if (c.second.inherit)
							continue;
						e->PushStyleColor(c.first, c.second.value);
					}
				}
				if (!e->v_inherit_all_styles)
				{
					for (auto& c : e->v_styles)
					{
						if (c.second.inherit)
							continue;
						if (c.second.type == StyleVarType::Float)
							e->PushStyleVar(c.first, c.second.value.Float);
						else if (c.second.type == StyleVarType::Vec2)
							e->PushStyleVar(c.first, c.second.value.Vec2);
					}
				}

				if (e->v_font.font)
					ImGui::PushFont(e->v_font.font);

				e->RenderHead();
				e->RenderInternal();
				e->RenderFoot();

				if (e->v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0 && need_disable_pop)
					ImGui::EndDisabled();
				if (e->v_font.font)
					ImGui::PopFont();
				PopColorAndStyles();


				ImGui::End();
			}
			ImGui::PopStyleVar(2);
		}
}

//special case keybinds?? unsure what if anything I want to do here, most binds are held by the workspace itself
void ImGuiElement::KeyBinds()
{

}


void ImGuiElement::ApplyResize(ImVec2 literal_size)
{
	if (literal_size.x < 0)
		literal_size.x = 0;
	if (literal_size.y < 0)
		literal_size.y = 0;
	if (v_size.type == Vec2Type::Absolute)
	{
		if (ImGui::GetIO().KeyShift)
		{
			v_size.value.x = literal_size.x;
			v_size.value.y = literal_size.x * v_aspect_ratio;
		}
		else
		{
			v_size.value = literal_size;
		}

	}
	else if (v_size.type == Vec2Type::Relative)
	{
		v_size.value.x = (literal_size.x / ContentRegionAvail.x) * 100;
		v_size.value.y = (literal_size.y / ContentRegionAvail.y) * 100;
	}
}

void ImGuiElement::ApplyDeltaResize(ImVec2 delta)
{
	if (delta.x == 0 && delta.y == 0)
		return;
	if (v_size.type == Vec2Type::Absolute)
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
		{
			v_size.value.x = last_size.x + delta.x;
			v_size.value.y = last_size.y + delta.x * v_aspect_ratio;
		}
		else
		{
			v_size.value = last_size + delta;
		}

	}
	else if (v_size.type == Vec2Type::Relative)
	{
		if (delta.x != 0)
			v_size.value.x += (delta.x / ContentRegionAvail.x) * 100;
		if (delta.y != 0)
			v_size.value.y += (delta.y / ContentRegionAvail.y) * 100;
	}
}

void ImGuiElement::ApplyDeltaPosDrag(ImVec2 delta)
{
	v_pos_dragging.value += delta;
}

void ImGuiElement::ApplyDeltaPos(ImVec2 delta)
{
	if (v_pos.type == Vec2Type::Absolute)
	{
		v_pos.value = last_position + delta;
	}
	else if (v_pos.type == Vec2Type::Relative)
	{
		v_pos.value.x = ((last_position.x + delta.x) / ContentRegionAvail.x) * 100;
		v_pos.value.y = ((last_position.y + delta.y) / ContentRegionAvail.y) * 100;
	}
}
void ImGuiElement::ApplyPos(ImVec2 literal_pos)
{
	if (v_pos.type == Vec2Type::Absolute)
	{
		v_pos.value = literal_pos;
	}
	else if (v_pos.type == Vec2Type::Relative)
	{
		v_pos.value.x = (literal_pos.x / ContentRegionAvail.x) * 100;
		v_pos.value.y = (literal_pos.y / ContentRegionAvail.y) * 100;
	}
}
bool isNeg(float val)
{
	return val < 0;
}



//void ImGuiElement::GetResizeRects()
//{
//	float delta_offset_outter = 5.0f;
//	float delta_offset_inner = -5.0f;
//	resize_rects[ResizeDirection::bottom_right] = ImRect(item_rect.Max.x+ delta_offset_inner, item_rect.Max.y + delta_offset_inner, item_rect.Max.x + delta_offset_outter, item_rect.Max.y + delta_offset_outter);
//	ImGui::GetForegroundDrawList()->AddRect(resize_rects[ResizeDirection::bottom_right].Min, resize_rects[ResizeDirection::bottom_right].Max, ImColor(1.0f, 0.0f, 1.0f, 1.0f), 0, 0, 2);
//}

bool ImGuiElement::Resize()
{
	//GetResizeRects();
	if ((v_property_flags & property_flags::no_resize) || !igd::active_workspace->CanSelect() || igd::active_workspace->dragging_select)
		return false;

	float delta_offset_outter = 8.0f;
	float delta_offset_inner = 3.0f;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	//ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	//ImVec2 current_size = ImVec2(g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.y - g.LastItemData.Rect.Min.y);
	ImVec2 mouse_delta_br = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_bl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_tr = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };
	ImVec2 mouse_delta_tl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };

	//if (this->v_type_id == (int)element_type::button)
	//{
	//	std::stringstream ss;
	//	ss << "mouse_delta_br: " << mouse_delta_br.x << " " << mouse_delta_br.y << std::endl;
	//	ss << "mouse_delta_bl: " << mouse_delta_bl.x << " " << mouse_delta_bl.y << std::endl;
	//	ss << "mouse_delta_tr: " << mouse_delta_tr.x << " " << mouse_delta_tr.y << std::endl;
	//	ss << "mouse_delta_tl: " << mouse_delta_tl.x << " " << mouse_delta_tl.y << std::endl;
	//	ImGui::Text(ss.str().c_str());
	//}
	bool is_mouse_hovering_br = fabs(mouse_delta_br.x) < delta_offset_outter && fabs(mouse_delta_br.y) < delta_offset_outter;
	bool is_mouse_hovering_r = fabs(mouse_delta_br.x) < delta_offset_outter && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_tr = fabs(mouse_delta_tr.x) < delta_offset_outter && fabs(mouse_delta_tr.y) < delta_offset_outter;
	bool is_mouse_hovering_l = fabs(mouse_delta_bl.x) < delta_offset_outter && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_b = fabs(mouse_delta_bl.y) < delta_offset_outter && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_t = fabs(mouse_delta_tr.y) < delta_offset_outter && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_bl = fabs(mouse_delta_bl.x) < delta_offset_outter && fabs(mouse_delta_bl.y) < delta_offset_outter;
	bool is_mouse_hovering_tl = fabs(mouse_delta_tl.x) < delta_offset_outter && fabs(mouse_delta_tl.y) < delta_offset_outter;

	//bool is_mouse_hovering_br = (isNeg(mouse_delta_br.x) && fabs(mouse_delta_br.x) < delta_offset_outter) && (fabs(mouse_delta_br.y) < delta_offset_outter);// mouse_delta_br.x < delta_offset_outter && (mouse_delta_br.y) < delta_offset_outter;
	//bool is_mouse_hovering_r = false;
	//bool is_mouse_hovering_tr = false;
	//bool is_mouse_hovering_l = false;
	//bool is_mouse_hovering_b = false;
	//bool is_mouse_hovering_t = false;
	//bool is_mouse_hovering_bl = false;
	//bool is_mouse_hovering_tl = false;

	if (resize_direction == ResizeDirection::none)
	{
		if (is_mouse_hovering_br)
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
		else if (is_mouse_hovering_tr)
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
		else if (is_mouse_hovering_bl)
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
		else if (is_mouse_hovering_tl)
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
		else if (is_mouse_hovering_r)
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
		else if (is_mouse_hovering_l)
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
		else if (is_mouse_hovering_b)
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
		else if (is_mouse_hovering_t)
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
	}

	if (ImGui::IsMouseClicked(0) && !igd::active_workspace->is_dragging && resize_direction == ResizeDirection::none)
	{
		if (is_mouse_hovering_br)
			resize_direction = ResizeDirection::bottom_right;
		else if (is_mouse_hovering_r)
			resize_direction = ResizeDirection::right;
		else if (is_mouse_hovering_tr)
			resize_direction = ResizeDirection::top_right;
		else if (is_mouse_hovering_l)
			resize_direction = ResizeDirection::left;
		else if (is_mouse_hovering_b)
			resize_direction = ResizeDirection::bottom;
		else if (is_mouse_hovering_t)
			resize_direction = ResizeDirection::top;
		else if (is_mouse_hovering_bl)
			resize_direction = ResizeDirection::bottom_left;
		else if (is_mouse_hovering_tl)
			resize_direction = ResizeDirection::top_left;
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && resize_direction != ResizeDirection::none)
	{
		mouse_drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
		did_resize = true;
		switch (resize_direction)
		{
		case ResizeDirection::top_right:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
			ApplyDeltaResize({ mouse_drag_delta.x,-mouse_drag_delta.y });// { last_size.x + mouse_drag_delta.x, last_size.y - mouse_drag_delta.y });
			ApplyDeltaPos({ 0, mouse_drag_delta.y });
			break;
		}
		case ResizeDirection::top_left:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
			ApplyDeltaResize(mouse_drag_delta * -1);// { last_size.x - mouse_drag_delta.x, last_size.y - mouse_drag_delta.y });
			ApplyDeltaPos(mouse_drag_delta);
			break;
		}
		case ResizeDirection::bottom_left:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
			ApplyDeltaResize({ -mouse_drag_delta.x, mouse_drag_delta.y });
			ApplyDeltaPos({ mouse_drag_delta.x, 0 });
			break;
		}
		case ResizeDirection::left:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
			ApplyDeltaResize({ -mouse_drag_delta.x, 0 });
			ApplyDeltaPos({ mouse_drag_delta.x, 0 });
			break;
		}
		case ResizeDirection::bottom_right:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
			ApplyDeltaResize(mouse_drag_delta);
			break;
		}
		case ResizeDirection::right:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
			ApplyDeltaResize({ mouse_drag_delta.x,0 });
			break;
		}
		case ResizeDirection::top:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
			ApplyDeltaResize({ 0, -mouse_drag_delta.y });
			ApplyDeltaPos({ 0,  mouse_drag_delta.y });
			break;
		}
		case ResizeDirection::bottom:
		{
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
			ApplyDeltaResize({ 0, mouse_drag_delta.y });
			break;
		}
		}
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
	}
	return resize_direction != ResizeDirection::none;
}

void ImGuiElement::Select()
{
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGuiWindow* window = g.CurrentWindow;

	if (this->drop_new_parent || igd::active_workspace->is_dragging || igd::active_workspace->dragging_select)
	{
		//std::cout << "drop_new_parent: " << (int)this->drop_new_parent << " is_dragging: " << (int)igd::active_workspace->is_dragging << " dragging select: " << (int)igd::active_workspace->dragging_select << " Can Select: " << (int)igd::active_workspace->CanSelect() << std::endl;
		return;
	}

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_None) && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && resize_direction == ResizeDirection::none)
		igd::active_workspace->SelectElement(this);
}

void ImGuiElement::DrawSelection()
{
	ImGui::GetWindowDrawList()->AddRect(item_rect.Min, item_rect.Max, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, 0, 2.0f);
}


void ImGuiElement::Interact()
{

	ImGuiContext& g = *GImGui;
	if (igd::active_workspace->interaction_mode == InteractionMode::designer)
	{
		for (auto& e : igd::active_workspace->selected_elements)
		{
			if (e == this)
			{

				if (Resize() || Drag())
					igd::active_workspace->is_interacting = true;
				else
					igd::active_workspace->is_interacting = false;
				DrawSelection();
				KeyBinds();
			}
		}

		if (g.MouseCursor == ImGuiMouseCursor_Hand || g.MouseCursor == ImGuiMouseCursor_Arrow || g.MouseCursor == ImGuiMouseCursor_TextInput)
			Select();

		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (did_resize)
			{
				PushUndo();
				did_resize = false;
			}
			resize_direction = ResizeDirection::none;
			if (did_move)
			{
				if (!drop_new_parent)
				{
					for (auto& e : igd::active_workspace->selected_elements)
					{
						ImVec2 final_pos = e->v_pos_dragging.value - e->v_parent->item_rect.Min + e->v_parent->v_scroll_position;
						if (e->v_pos.type == Vec2Type::Absolute && e->v_parent)
							e->v_pos.value = final_pos;
						else
						{
							e->v_pos.value.x = (final_pos.x / ContentRegionAvail.x) * 100;
							e->v_pos.value.y = (final_pos.y / ContentRegionAvail.y) * 100;
						}
					}
				}
				PushUndo();
				did_move = false;
			}
		}
	}
}