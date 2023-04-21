#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962
#endif

//
//RectSide getNearestSide(const ImVec2& center, const ImVec2& point) {
//	// Calculate the distance between the point and the center of the rectangle.
//	const float dx = point.x - center.x;
//	const float dy = point.y - center.y;
//	const float distanceSquared = dx * dx + dy * dy;
//
//	// Calculate the angle between the x-axis and the vector from the center to the point.
//	const float angle = atan2(dy, dx);
//	ImGui::Text("Angle: %f, Pi_4: %f", angle, M_PI_4);
//	// Determine which side the point is closest to based on the angle.
//	if (angle > -M_PI_4 && angle <= M_PI_4) {
//		return RectSide::Right;
//	}
//	else if (angle > M_PI_4 && angle <= 3.5 * M_PI_4) {
//		return RectSide::Bottom;
//	}
//	else if (angle > -3.5 * M_PI_4 && angle <= -M_PI_4) {
//		return RectSide::Top;
//	}
//	else {
//		return RectSide::Left;
//	}
//}

void DrawDashedLine(ImDrawList* drawList, ImVec2 p1, ImVec2 p2, ImU32 color, float thickness, float dashSize)
{
	const ImVec2 diff = p2 - p1;
	const float length = sqrtf(ImLengthSqr(diff));
	const ImVec2 dir = diff * (1.f / length);
	const int numDashes = static_cast<int>(length / dashSize + 0.5f);
	const float remainingLength = length - numDashes * dashSize;
	const float dashSpacing = remainingLength / (numDashes - 1);

	for (int i = 0; i < numDashes; ++i)
	{
		if (i % 2 == 0)
			continue;
		const float start = i * dashSize + fminf(i * dashSpacing, remainingLength);
		const float end = fminf(start + dashSize, length);
		const ImVec2 dashStart = p1 + dir * start;
		const ImVec2 dashEnd = p1 + dir * end;
		drawList->AddLine(dashStart, dashEnd, color, thickness);
	}
}

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


void ImGuiElement::DragSnap()
{
//	for (auto& e : igd::active_workspace->selected_elements)
//	{
		static ImVec2 previous_drag_delta = ImGui::GetMouseDragDelta();
		const float side_indicator_size = 2;
		const float line_offset = 5;
		const float max_snap_dist = 5; //distance away before it snaps to pos
		const float snap_dist = 20; //amount of movement before it unsnaps
		const float snap_dist_draw = 50;
		ImGuiElement* nearest = nullptr;
		bool is_larger_group = false;

		if (igd::active_workspace->selected_elements.size() > 1)
		{
			ImGui::GetForegroundDrawList()->AddRectFilled(this->GetItemRect().Min, this->GetItemRect().Max, ImColor(255, 255, 255, 15));

		}
		
		nearest = igd::GetNearestElement(this);
		if (nearest && igd::GetDistance(this->GetPos(), nearest->GetPos()) < 600)
		{
			ImGui::GetForegroundDrawList()->AddRectFilled(nearest->GetItemRect().Min, nearest->GetItemRect().Max, ImColor(255, 255, 255, 15));
			//RectSide side = getNearestSide(nearest->GetPos() + ImVec2(nearest->GetRawSize() / 2), this->GetPos() + ImVec2(this->GetRawSize() / 2));// , nearest->GetItemRect().Max.x - nearest->GetItemRect().Min.x, nearest->GetItemRect().Max.y - nearest->GetItemRect().Min.y);
			RectSide side = igd::getNearestSide(nearest->GetItemRect(), { this->GetPos(),this->GetPos() + this->GetRawSize() }, FLT_MAX);// , nearest->GetItemRect().Max.x - nearest->GetItemRect().Min.x, nearest->GetItemRect().Max.y - nearest->GetItemRect().Min.y);
			bool reset_snap = false;
			switch (side)
			{
				case RectSide::Bottom:
				{
					ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->GetItemRect().Min.x,nearest->GetItemRect().Max.y }, { nearest->GetItemRect().Max.x,nearest->GetItemRect().Max.y + side_indicator_size }, ImColor(255, 0, 255, 150), 1.f, 0);
					float l = this->GetPos().x - nearest->GetItemRect().Min.x;
					float r = (this->GetPos().x + this->GetRawSize().x) - nearest->GetItemRect().Max.x;
					float m = (this->GetPos().x + (this->GetRawSize().x/2)) - (nearest->GetItemRect().Min.x +(nearest->GetRawSize().x/2));

					if (fabs(l) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Min.x,this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(m) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),nearest->GetItemRect().Min.y - line_offset }, { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(r) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Max.x,this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(255, 255, 255, 60), 3.f, 5.f);

					if (fabs(l) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x;
						reset_snap = true;
					}
					else if (fabs(m) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x+((nearest->GetRawSize().x - this->GetRawSize().x) / 2);
						reset_snap = true;
					}
					else if (fabs(r) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Max.x-this->GetRawSize().x;
						reset_snap = true;
					}
					else
						this->SnapDist = 0;

					break;
				}
				case RectSide::Top:
				{
					ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->GetItemRect().Min.x,nearest->GetItemRect().Max.y }, { nearest->GetItemRect().Max.x,nearest->GetItemRect().Max.y + side_indicator_size }, ImColor(255, 0, 255, 150), 1.f, 0);
					float l = this->GetPos().x - nearest->GetItemRect().Min.x;
					float r = (this->GetPos().x + this->GetRawSize().x) - nearest->GetItemRect().Max.x;
					float m = (this->GetPos().x + (this->GetRawSize().x / 2)) - (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2));

					if (fabs(l) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Min.x,this->GetPos().y  - line_offset }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(m) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),nearest->GetItemRect().Min.y - line_offset }, { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),this->GetPos().y - line_offset }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(r) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Max.x,this->GetPos().y - line_offset }, ImColor(255, 255, 255, 60), 3.f, 5.f);

					if (fabs(l) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x;
						reset_snap = true;
					}
					else if (fabs(m) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x + ((nearest->GetRawSize().x - this->GetRawSize().x) / 2);
						reset_snap = true;
					}
					else if (fabs(r) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Max.x - this->GetRawSize().x;
						reset_snap = true;
					}
					else
						this->SnapDist = 0;

					break;
				}
				case RectSide::Left:
				{
					
					ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->GetItemRect().Min.x,nearest->GetItemRect().Max.y }, { nearest->GetItemRect().Max.x,nearest->GetItemRect().Max.y + side_indicator_size }, ImColor(255, 0, 255, 150), 1.f, 0);
					float t = this->GetPos().y - nearest->GetItemRect().Min.y;
					float b = (this->GetPos().y + this->GetRawSize().y) - nearest->GetItemRect().Max.y;
					float m = (this->GetPos().y + (this->GetRawSize().y / 2)) - (nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2));
					if (fabs(t) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Min.y }, { this->GetPos().x - line_offset, nearest->GetItemRect().Min.y }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(m) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, { this->GetPos().x - line_offset, nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(b) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Max.y}, { this->GetPos().x - line_offset, nearest->GetItemRect().Max.y }, ImColor(255, 255, 255, 60), 3.f, 5.f);

					if (fabs(t) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y;
						reset_snap = true;
					}
					else if (fabs(m) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y + ((nearest->GetRawSize().y - this->GetRawSize().y) / 2);
						reset_snap = true;
					}
					else if (fabs(b) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Max.y - this->GetRawSize().y;
						reset_snap = true;
					}
					else
						this->SnapDist = 0;

					break;
				}
				case RectSide::Right:
				{
					ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->GetItemRect().Min.x,nearest->GetItemRect().Max.y }, { nearest->GetItemRect().Max.x,nearest->GetItemRect().Max.y + side_indicator_size }, ImColor(255, 0, 255, 150), 1.f, 0);
					float t = this->GetPos().y - nearest->GetItemRect().Min.y;
					float b = (this->GetPos().y + this->GetRawSize().y) - nearest->GetItemRect().Max.y;
					float m = (this->GetPos().y + (this->GetRawSize().y / 2)) - (nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2));
					if (fabs(t) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Min.y }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Min.y}, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(m) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, ImColor(255, 255, 255, 60), 3.f, 5.f);
					if (fabs(b) < snap_dist_draw)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Max.y }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Max.y }, ImColor(255, 255, 255, 60), 3.f, 5.f);

					if (fabs(t) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y;
						reset_snap = true;
					}
					if (fabs(m) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y + ((nearest->GetRawSize().y - this->GetRawSize().y) / 2);
						reset_snap = true;
					}
					if (fabs(b) < snap_dist)
					{
						this->SnapDist += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist) < max_snap_dist)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Max.y - this->GetRawSize().y;
						reset_snap = true;
					}
					else
						this->SnapDist = 0;
					break;
				}
			}
			if (reset_snap)
			{
				if (ImGui::GetMouseDragDelta().x < 0 && previous_drag_delta.x > 0)
					this->SnapDist = 0;
				if (ImGui::GetMouseDragDelta().x > 0 && previous_drag_delta.x < 0)
					this->SnapDist = 0;
			}

			////ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x,nearest->item_rect.Max.y }, { nearest->item_rect.Max.x,nearest->item_rect.Max.y + 5 }, ImColor(255, 0, 255, 150), 1.f, 0);
			//std::stringstream ss;
			//ss << "Dist: " << igd::GetDistance(e->GetPos(), nearest->GetPos());
			////ImGui::GetForegroundDrawList()->AddRectFilled({ nearest->item_rect.Min.x,nearest->item_rect.Min.y - 5 }, { nearest->item_rect.Max.x,nearest->item_rect.Min.y }, ImColor(255, 0, 255, 150), 1.f, 0);
			//ImGui::GetForegroundDrawList()->AddRectFilled(nearest->item_rect.Min, nearest->item_rect.Max, ImColor(0, 0, 0, 255), 1.f, 0);
			//ImGui::GetForegroundDrawList()->AddCircleFilled(nearest->GetPos() + ImVec2(nearest->GetSize() / 2), 5, ImColor(255, 0, 255, 150), 10);
			//ImGui::GetForegroundDrawList()->AddText(nearest->item_rect.Min, ImColor(0.0f, 1.0f, 0.0f, 1.0f), ss.str().c_str());
		}
		if (ImGui::GetMouseDragDelta().x !=0 || ImGui::GetMouseDragDelta().y!=0)
			previous_drag_delta = ImGui::GetMouseDragDelta();
}


bool ImGuiElement::Drag()
{
	ImGuiContext& g = *GImGui;

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(0) && resize_direction == ResizeDirection::none && !igd::active_workspace->is_dragging)
	{
		this->v_pos_dragging.value = { this->item_rect.Min.x, this->item_rect.Min.y };
		igd::active_workspace->is_dragging = true;
		for (auto&e : igd::active_workspace->selected_elements)
			e->v_is_dragging = true;
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && igd::active_workspace->is_dragging)
	{
		igd::active_workspace->ResetSelectTimeout();
		for (auto& e : igd::active_workspace->selected_elements)
			e->v_is_dragging = false;
	}

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && this->v_is_dragging)
	{
		did_move = true;
		g.MouseCursor = ImGuiMouseCursor_ResizeAll;
		this->ApplyDeltaPosDrag(ImGui::GetMouseDragDelta());
		/*for (auto& e : igd::active_workspace->selected_elements)
		{
			e->ApplyDeltaPosDrag(ImGui::GetMouseDragDelta());
		}*/
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