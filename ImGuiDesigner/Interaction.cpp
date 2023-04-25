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


ImVec2 ImGuiElement::GetAverageSpacing()
{
		ImGuiElement* parent = this->v_parent;
		if (!parent)
			parent = igd::active_workspace->basic_workspace_element;
			
		std::vector<ImRect> vrects;
		for (auto& e : parent->children)
		{
			if (e == this)
				continue;
			if (!e->delete_me)
				vrects.push_back(e->GetItemRect());
		}

		ImRect* rects = vrects.data();
		int numRects = vrects.size();
		// Sort the rectangles by their y-coordinates
		std::sort(rects, rects + numRects, [](const ImRect& a, const ImRect& b) { return a.Min.y < b.Min.y; });

		float totalXSpacing = 0.0f;
		float totalYSpacing = 0.0f;
		int numXSpacings = 0;
		int numYSpacings = 0;

		// Iterate over each row of rectangles
		for (int i = 0; i < numRects; )
		{
			// Find the range of rectangles in the current row (i.e., those with the same y-coordinate)
			int j = i + 1;
			while (j < numRects && rects[j].Min.y == rects[i].Min.y) { j++; }

			// Sort the rectangles in the current row by their x-coordinates
			std::sort(rects + i, rects + j, [](const ImRect& a, const ImRect& b) { return a.Min.x < b.Min.x; });

			// Calculate the spacing between adjacent rectangles in the current row
			for (int k = i + 1; k < j; k++)
			{
				float spacing = rects[k].Min.x - rects[k - 1].Max.x;
				totalXSpacing += spacing;
				numXSpacings++;
			}

			// Calculate the spacing between the current row and the previous row (if there is one)
			if (i > 0)
			{
				float spacing = rects[i].Min.y - rects[i - 1].Max.y;
				totalYSpacing += spacing;
				numYSpacings++;
			}

			// Move on to the next row
			i = j;
		}

		// Calculate the average spacings
		float averageXSpacing = 0.0f;
		float averageYSpacing = 0.0f;
		if (numXSpacings > 0) { averageXSpacing = totalXSpacing / numXSpacings; }
		if (numYSpacings > 0) { averageYSpacing = totalYSpacing / numYSpacings; }

		return ImVec2(averageXSpacing, averageYSpacing);
}


std::unordered_map<VerticalDistance, float> ImGuiElement::GetVerticalDistance(ImGuiElement* nearest, bool use_abs)
{
	std::unordered_map<VerticalDistance, float> rval;
	rval[VerticalDistance::TopToTop] = (this->GetPos().y - nearest->GetItemRect().Min.y);
	rval[VerticalDistance::TopToBottom] = (this->GetPos().y - nearest->GetItemRect().Max.y);
	rval[VerticalDistance::BottomToBottom] = ((this->GetPos().y + this->GetRawSize().y) - nearest->GetItemRect().Max.y);
	rval[VerticalDistance::BottomToTop] = ((this->GetPos().y + this->GetRawSize().y) - nearest->GetItemRect().Min.y);
	rval[VerticalDistance::MiddleToMiddle] = ((this->GetPos().y + (this->GetRawSize().y / 2)) - (nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2)));
	rval[VerticalDistance::TopToMiddle] = ((this->GetPos().y) - (nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2)));
	rval[VerticalDistance::BottomToMiddle] = ((this->GetPos().y + this->GetRawSize().y) - (nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2)));
	if (use_abs)
	{
		for (auto& e : rval)
			e.second = fabs(e.second);
	}
	return rval;
}

std::unordered_map<HorizontalDistance, float> ImGuiElement::GetHorizontalDistance(ImGuiElement* nearest, bool use_abs)
{
	std::unordered_map<HorizontalDistance, float> rval;
	rval[HorizontalDistance::LeftToLeft] = (this->GetPos().x - nearest->GetItemRect().Min.x);
	rval[HorizontalDistance::LeftToRight] = (this->GetPos().x - nearest->GetItemRect().Max.x);
	rval[HorizontalDistance::RightToRight] = ((this->GetPos().x + this->GetRawSize().x) - nearest->GetItemRect().Max.x);
	rval[HorizontalDistance::RightToLeft] = ((this->GetPos().x + this->GetRawSize().x) - nearest->GetItemRect().Min.x);
	rval[HorizontalDistance::MiddleToMiddle] = ((this->GetPos().x + (this->GetRawSize().x / 2)) - (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)));
	rval[HorizontalDistance::LeftToMiddle] = ((this->GetPos().x) - (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)));
	rval[HorizontalDistance::RightToMiddle] = ((this->GetPos().x+this->GetRawSize().x) - (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)));
	if (use_abs)
	{
		for (auto& e : rval)
			e.second = fabs(e.second);
	}
	return rval;
}
void ImGuiElement::SnapVertical(std::unordered_map<VerticalDistance, float>& distances, ImGuiElement* nearest, float mouse_delta_threshold, float distance_threshold, bool& reset_snap)
{
	if (distances[VerticalDistance::TopToTop] < distance_threshold)
	{
		this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
		if (fabs(this->SnapDist.y) < mouse_delta_threshold)
			this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y;
		reset_snap = true;
	}
	else if (distances[VerticalDistance::BottomToTop] < distance_threshold)
	{
		this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
		if (fabs(this->SnapDist.y) < mouse_delta_threshold)
			this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y-this->GetRawSize().y;
		reset_snap = true;
	}
	else if (distances[VerticalDistance::MiddleToMiddle] < distance_threshold)
	{
		this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
		if (fabs(this->SnapDist.y) < mouse_delta_threshold)
			this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y + ((nearest->GetRawSize().y - this->GetRawSize().y) / 2);
		reset_snap = true;
	}
	else if (distances[VerticalDistance::TopToBottom] < distance_threshold)
	{
		this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
		if (fabs(this->SnapDist.y) < mouse_delta_threshold)
			this->v_pos_dragging.value.y = nearest->GetItemRect().Max.y;
		reset_snap = true;
	}
	else if (distances[VerticalDistance::BottomToBottom] < distance_threshold)
	{
		this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
		if (fabs(this->SnapDist.y) < mouse_delta_threshold)
			this->v_pos_dragging.value.y = nearest->GetItemRect().Max.y - this->GetRawSize().y;
		reset_snap = true;
	}
}
void ImGuiElement::SnapHorizontal(std::unordered_map<HorizontalDistance, float>& distances, ImGuiElement* nearest, float mouse_delta_threshold, float distance_threshold, bool& reset_snap)
{
	if (distances[HorizontalDistance::LeftToLeft] < distance_threshold)
	{
		this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
		if (fabs(this->SnapDist.x) < mouse_delta_threshold)
			this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x;
		reset_snap = true;
	}
	else if (distances[HorizontalDistance::LeftToRight] < distance_threshold)
	{
		this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
		if (fabs(this->SnapDist.x) < mouse_delta_threshold)
			this->v_pos_dragging.value.x = nearest->GetItemRect().Max.x;
		reset_snap = true;
	}
	else if (distances[HorizontalDistance::MiddleToMiddle] < distance_threshold)
	{
		this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
		if (fabs(this->SnapDist.x) < mouse_delta_threshold)
			this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x + ((nearest->GetRawSize().x - this->GetRawSize().x) / 2);
		reset_snap = true;
	}
	else if (distances[HorizontalDistance::RightToRight] < distance_threshold)
	{
		this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
		if (fabs(this->SnapDist.x) < mouse_delta_threshold)
			this->v_pos_dragging.value.x = nearest->GetItemRect().Max.x - this->GetRawSize().x;
		reset_snap = true;
	}
	else if (distances[HorizontalDistance::RightToLeft] < distance_threshold)
	{
		this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
		if (fabs(this->SnapDist.x) < mouse_delta_threshold)
			this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x - this->GetRawSize().x;
		reset_snap = true;
	}
}

void ImGuiElement::DragSnap()
{
//	for (auto& e : igd::active_workspace->selected_elements)
//	{
		static ImVec2 previous_drag_delta = ImGui::GetMouseDragDelta();
		const float side_indicator_size = 2;
		const float line_offset = 5;
		const float mouse_delta_threshold = 15; //maximum amount of mouse movement before it unsnaps
		const float distance_threshold = 5; //distance from the snap location before it snaps the element to it
		const float snap_dist_draw = 100;
		ImGuiElement* nearest = nullptr;
		bool is_larger_group = false;

		if (igd::active_workspace->selected_elements.size() > 1) //render a rectangle that encases all of the currently selected elements
		{
			ImGui::GetForegroundDrawList()->AddRectFilled(this->GetItemRect().Min, this->GetItemRect().Max, ImColor(255, 255, 255, 15));
		}
		
		nearest = igd::GetNearestElement(this, true);
		if (nearest)// && igd::GetDistance(this->GetPos(), nearest->GetPos()) < 600)
		{
			ImGui::GetForegroundDrawList()->AddRectFilled(nearest->GetItemRect().Min, nearest->GetItemRect().Max, ImColor(255, 255, 255, 15));
			RectSide side = igd::getNearestSide(nearest->GetItemRect(), { this->GetPos(),this->GetPos() + this->GetRawSize() }, FLT_MAX);// , nearest->GetItemRect().Max.x - nearest->GetItemRect().Min.x, nearest->GetItemRect().Max.y - nearest->GetItemRect().Min.y);
			ImVec2 spacing = this->GetAverageSpacing();
			bool reset_snap = false;
			std::unordered_map<VerticalDistance, float> distances_vertical = this->GetVerticalDistance(nearest);
			std::unordered_map<HorizontalDistance, float> distances_horizontal = this->GetHorizontalDistance(nearest);
			this->SnapVertical(distances_vertical, nearest, mouse_delta_threshold, distance_threshold, reset_snap);
			this->SnapHorizontal(distances_horizontal, nearest, mouse_delta_threshold, distance_threshold, reset_snap);

			ImVec2 this_mid = { this->GetItemRect().Min.x + ((this->GetItemRect().Max.x - this->GetItemRect().Min.x) / 2) , this->GetItemRect().Min.y + ((this->GetItemRect().Max.y - this->GetItemRect().Min.y) / 2) };
			ImVec2 nearest_mid = { nearest->GetItemRect().Min.x + ((nearest->GetItemRect().Max.x - nearest->GetItemRect().Min.x) / 2) , nearest->GetItemRect().Min.y + ((nearest->GetItemRect().Max.y - nearest->GetItemRect().Min.y) / 2) };

			if (this_mid.y >= nearest_mid.y)
			{
				//bottom
				if (distances_horizontal[HorizontalDistance::LeftToLeft] < snap_dist_draw || distances_horizontal[HorizontalDistance::RightToLeft] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Min.x,this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),nearest->GetItemRect().Max.y  }, { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::RightToRight] < snap_dist_draw || distances_horizontal[HorizontalDistance::LeftToRight] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Max.x,this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}
			//top
			else if (this_mid.y < nearest_mid.y)
			{
				if (distances_horizontal[HorizontalDistance::LeftToLeft] < snap_dist_draw || distances_horizontal[HorizontalDistance::RightToLeft] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Min.x,this->GetPos().y - line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),nearest->GetItemRect().Min.y - line_offset }, { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),this->GetPos().y - line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::RightToRight] < snap_dist_draw || distances_horizontal[HorizontalDistance::LeftToRight] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Max.x,this->GetPos().y - line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}

			if (this_mid.x <= nearest_mid.x)
			{
				//left
				if (distances_vertical[VerticalDistance::TopToTop] < snap_dist_draw || distances_vertical[VerticalDistance::BottomToTop] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Min.y }, { this->GetPos().x - line_offset, nearest->GetItemRect().Min.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, { this->GetPos().x - line_offset, nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::BottomToBottom] < snap_dist_draw || distances_vertical[VerticalDistance::TopToBottom] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Max.y }, { this->GetPos().x - line_offset, nearest->GetItemRect().Max.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}

			else if (this_mid.x > nearest_mid.x)
			{
				//right
				if (distances_vertical[VerticalDistance::TopToTop] < snap_dist_draw || distances_vertical[VerticalDistance::BottomToTop] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Min.y }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Min.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::BottomToBottom] < snap_dist_draw || distances_vertical[VerticalDistance::TopToBottom] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Max.y }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Max.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}
			switch (side)
			{
				case RectSide::Bottom:
				{
					float Spacing = (this->GetPos().y - (nearest->GetItemRect().Max.y + spacing.y));
					if (spacing.y>10)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x, nearest->GetItemRect().Max.y + spacing.y }, { nearest->GetItemRect().Max.x, nearest->GetItemRect().Max.y + spacing.y }, ImColor(255, 0, 0, 60), 3.f, 5.f);
					if (fabs(Spacing) < distance_threshold)
					{
						this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist.y) < mouse_delta_threshold)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Max.y + spacing.y;
						reset_snap = true;
					}


					break;
				}
				case RectSide::Top:
				{
					float Spacing = ((this->GetPos().y + this->GetRawSize().y) - (nearest->GetItemRect().Min.y - spacing.y));
					if (spacing.y > 10)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x, nearest->GetItemRect().Min.y - spacing.y }, { nearest->GetItemRect().Max.x, nearest->GetItemRect().Min.y - spacing.y }, ImColor(255, 0, 0, 60), 3.f, 5.f);
					if (fabs(Spacing) < distance_threshold)
					{
						this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
						if (fabs(this->SnapDist.y) < mouse_delta_threshold)
							this->v_pos_dragging.value.y = nearest->GetItemRect().Min.y - spacing.y - this->GetRawSize().y;
						reset_snap = true;
					}


					break;
				}
				case RectSide::Left:
				{
					float Spacing = (this->GetPos().x - (nearest->GetItemRect().Min.x - spacing.x));
					if (spacing.y > 10)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x-spacing.x, nearest->GetItemRect().Max.y}, { nearest->GetItemRect().Min.x - spacing.x, nearest->GetItemRect().Min.y }, ImColor(255, 0, 0, 60), 3.f, 5.f);
					if (fabs(Spacing) < distance_threshold)
					{
						this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist.x) < mouse_delta_threshold)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Min.x - spacing.x;
						reset_snap = true;
					}


					break;
				}
				case RectSide::Right:
				{
					float Spacing = (this->GetPos().x - (nearest->GetItemRect().Max.x + spacing.x));
					if (spacing.y > 10)
						DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x + spacing.x, nearest->GetItemRect().Max.y }, { nearest->GetItemRect().Max.x + spacing.x, nearest->GetItemRect().Min.y }, ImColor(255, 0, 0, 60), 3.f, 5.f);
					if (fabs(Spacing) < distance_threshold)
					{
						this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
						if (fabs(this->SnapDist.x) < mouse_delta_threshold)
							this->v_pos_dragging.value.x = nearest->GetItemRect().Max.x + spacing.x;
						reset_snap = true;
					}

					break;
				}
			}
			if (!reset_snap)
				this->SnapDist = { 0,0 };
			if (reset_snap)
			{
				if (ImGui::GetMouseDragDelta().x < 0 && previous_drag_delta.x > 0)
					this->SnapDist = { 0,0 };
				if (ImGui::GetMouseDragDelta().x > 0 && previous_drag_delta.x < 0)
					this->SnapDist = { 0,0 };
				if (ImGui::GetMouseDragDelta().y < 0 && previous_drag_delta.y > 0)
					this->SnapDist = { 0,0 };
				if (ImGui::GetMouseDragDelta().y > 0 && previous_drag_delta.y < 0)
					this->SnapDist = { 0,0 };
			}

		}
		if (ImGui::GetMouseDragDelta().x !=0 || ImGui::GetMouseDragDelta().y!=0)
			previous_drag_delta = ImGui::GetMouseDragDelta();
}


bool ImGuiElement::Drag()
{
	ImGuiContext& g = *GImGui;
	if (igd::active_workspace->dragging_select || !igd::active_workspace->is_focused)
		return false;

	//std::cout << "Dragging: " << ImGui::IsMouseDragging(ImGuiMouseButton_Left) << " Dragging State: " << igd::active_workspace->is_dragging << " hovered: " << ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) << std::endl;
	
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && resize_direction == ResizeDirection::none && !igd::active_workspace->is_dragging)
	{
		std::cout << "Dragging" << std::endl;
		for (auto& e : igd::active_workspace->selected_elements)
		{
			e->v_is_dragging = true;
			e->v_pos_dragging.value = { e->item_rect.Min.x, e->item_rect.Min.y };
		}
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && igd::active_workspace->is_dragging)
	{
		igd::active_workspace->ResetSelectTimeout();
		for (auto& e : igd::active_workspace->selected_elements)
			e->v_is_dragging = false;
	}

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && this->v_is_dragging)
	{
		igd::active_workspace->is_dragging = true;
		did_move = true;
		g.MouseCursor = ImGuiMouseCursor_ResizeAll;
		this->ApplyDeltaPosDrag(ImGui::GetMouseDragDelta());
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
			int color_pops = 0;
			int style_pops = 0;
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
						ImGui::PushStyleColor(c.first, c.second.value);
						color_pops++;
					}
				}
				if (!e->v_inherit_all_styles)
				{
					for (auto& c : e->v_styles)
					{
						if (c.second.inherit)
							continue;
						if (c.second.type == StyleVarType::Float)
							ImGui::PushStyleVar(c.first, c.second.value.Float);
						else if (c.second.type == StyleVarType::Vec2)
							ImGui::PushStyleVar(c.first, c.second.value.Vec2);
						style_pops++;
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
				if (style_pops>0)
					ImGui::PopStyleVar(style_pops);
				if (color_pops>0)
					ImGui::PopStyleColor(color_pops);
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
			v_size.value.y = last_size.y + (delta.x * v_aspect_ratio);
			last_size = v_size.value;
		}
		else
		{
			v_size.value = last_size + delta;
			last_size = v_size.value;
		}

	}
	else if (v_size.type == Vec2Type::Relative)
	{
		if (delta.x != 0)
			v_size.value.x += (delta.x / ContentRegionAvail.x) * 100;
		if (delta.y != 0)
			v_size.value.y += (delta.y / ContentRegionAvail.y) * 100;

		last_size += delta;
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
		last_position = v_pos.value;
	}
	else if (v_pos.type == Vec2Type::Relative)
	{
		last_position += delta;
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


bool ImGuiElement::ResizeSnap(ResizeDirection resize_direction)
{
	ImGuiContext& g = *GImGui;
	static ImVec2 previous_drag_delta = ImGui::GetMouseDragDelta();
	const float side_indicator_size = 2;
	const float line_offset = 5;
	const float mouse_delta_threshold = 15; //maximum amount of mouse movement before it unsnaps
	const float distance_threshold = 7; //distance from the snap location before it snaps the element to it
	const float snap_dist_draw = 1000;
	bool rval = false;
	ImGuiElement* nearest = igd::GetNearestElement(this, true);
	this->v_is_dragging = false;
	if (nearest)
	{
		bool reset_snap = false;
		ImGui::GetForegroundDrawList()->AddRectFilled(nearest->GetItemRect().Min, nearest->GetItemRect().Max, ImColor(255, 255, 255, 15));
		RectSide side = igd::getNearestSide(nearest->GetItemRect(), { this->GetPos(),this->GetPos() + this->GetRawSize() }, FLT_MAX);
		ImVec2 this_mid = { this->GetItemRect().Min.x + ((this->GetItemRect().Max.x - this->GetItemRect().Min.x) / 2) , this->GetItemRect().Min.y + ((this->GetItemRect().Max.y - this->GetItemRect().Min.y) / 2) };
		ImVec2 nearest_mid = { nearest->GetItemRect().Min.x + ((nearest->GetItemRect().Max.x - nearest->GetItemRect().Min.x) / 2) , nearest->GetItemRect().Min.y + ((nearest->GetItemRect().Max.y - nearest->GetItemRect().Min.y) / 2) };
		std::unordered_map<VerticalDistance, float> distances_vertical = this->GetVerticalDistance(nearest);
		std::unordered_map<HorizontalDistance, float> distances_horizontal = this->GetHorizontalDistance(nearest);
		if (resize_direction == ResizeDirection::left || resize_direction == ResizeDirection::right)
		{
			if (this_mid.y >= nearest_mid.y)
			{
				//bottom
				if (distances_horizontal[HorizontalDistance::LeftToLeft] < snap_dist_draw || distances_horizontal[HorizontalDistance::RightToLeft] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Min.x,this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),nearest->GetItemRect().Max.y }, { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::RightToRight] < snap_dist_draw || distances_horizontal[HorizontalDistance::LeftToRight] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Max.x,this->GetPos().y + this->GetRawSize().y + line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}
			//top
			else if (this_mid.y < nearest_mid.y)
			{
				if (distances_horizontal[HorizontalDistance::LeftToLeft] < snap_dist_draw || distances_horizontal[HorizontalDistance::RightToLeft] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Min.x,this->GetPos().y - line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),nearest->GetItemRect().Min.y - line_offset }, { (nearest->GetItemRect().Min.x + (nearest->GetRawSize().x / 2)),this->GetPos().y - line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_horizontal[HorizontalDistance::RightToRight] < snap_dist_draw || distances_horizontal[HorizontalDistance::LeftToRight] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x,nearest->GetItemRect().Min.y - line_offset }, { nearest->GetItemRect().Max.x,this->GetPos().y - line_offset }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}
		}
		if (resize_direction == ResizeDirection::top || resize_direction == ResizeDirection::bottom)
		{
			if (this_mid.x <= nearest_mid.x)
			{
				//left
				if (distances_vertical[VerticalDistance::TopToTop] < snap_dist_draw || distances_vertical[VerticalDistance::BottomToTop] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Min.y }, { this->GetPos().x - line_offset, nearest->GetItemRect().Min.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, { this->GetPos().x - line_offset, nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::BottomToBottom] < snap_dist_draw || distances_vertical[VerticalDistance::TopToBottom] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Min.x + line_offset,nearest->GetItemRect().Max.y }, { this->GetPos().x - line_offset, nearest->GetItemRect().Max.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}
			else if (this_mid.x > nearest_mid.x)
			{
				//right
				if (distances_vertical[VerticalDistance::TopToTop] < snap_dist_draw || distances_vertical[VerticalDistance::BottomToTop] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Min.y }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Min.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::MiddleToMiddle] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Min.y + (nearest->GetRawSize().y / 2) }, ImColor(66, 150, 250, 102), 3.f, 5.f);
				if (distances_vertical[VerticalDistance::BottomToBottom] < snap_dist_draw || distances_vertical[VerticalDistance::TopToBottom] < snap_dist_draw)
					DrawDashedLine(ImGui::GetForegroundDrawList(), { nearest->GetItemRect().Max.x - line_offset,nearest->GetItemRect().Max.y }, { this->GetPos().x + this->GetRawSize().x + line_offset, nearest->GetItemRect().Max.y }, ImColor(66, 150, 250, 102), 3.f, 5.f);
			}
		}
		distances_horizontal = this->GetHorizontalDistance(nearest,false);
		distances_vertical = this->GetVerticalDistance(nearest,false);
		switch (resize_direction)
		{
			case ResizeDirection::left:
			{
				if (fabs(distances_horizontal[HorizontalDistance::LeftToLeft]) < distance_threshold)
				{
					this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
					if (fabs(this->SnapDist.x) < mouse_delta_threshold)
					{
						ApplyDeltaPos({ -distances_horizontal[HorizontalDistance::LeftToLeft], 0 });
						ApplyDeltaResize({ distances_horizontal[HorizontalDistance::LeftToLeft],0 });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_horizontal[HorizontalDistance::LeftToRight]) < distance_threshold)
				{
					this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
					if (fabs(this->SnapDist.x) < mouse_delta_threshold)
					{
						ApplyDeltaPos({ -distances_horizontal[HorizontalDistance::LeftToRight], 0 });
						ApplyDeltaResize({ distances_horizontal[HorizontalDistance::LeftToRight],0 });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_horizontal[HorizontalDistance::LeftToMiddle]) < distance_threshold)
				{
					this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
					if (fabs(this->SnapDist.x) < mouse_delta_threshold)
					{
						ApplyDeltaPos({ -distances_horizontal[HorizontalDistance::LeftToMiddle], 0 });
						ApplyDeltaResize({ distances_horizontal[HorizontalDistance::LeftToMiddle],0 });
						rval = true;
					}
					reset_snap = true;
				}
				else
					this->SnapDist.x = 0;
				break;
			}
			case ResizeDirection::right:
			{
				if (fabs(distances_horizontal[HorizontalDistance::RightToRight]) < distance_threshold)
				{
					this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
					if (fabs(this->SnapDist.x) < mouse_delta_threshold)
					{
						ApplyDeltaResize({ -distances_horizontal[HorizontalDistance::RightToRight],0 });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_horizontal[HorizontalDistance::RightToMiddle]) < distance_threshold)
				{
					this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
					if (fabs(this->SnapDist.x) < mouse_delta_threshold)
					{
						ApplyDeltaResize({ -distances_horizontal[HorizontalDistance::RightToMiddle],0 });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_horizontal[HorizontalDistance::RightToLeft]) < distance_threshold)
				{
					this->SnapDist.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x;
					if (fabs(this->SnapDist.x) < mouse_delta_threshold)
					{
						ApplyDeltaResize({ -distances_horizontal[HorizontalDistance::RightToLeft],0 });
						rval = true;
					}
					reset_snap = true;
				}
				else
					this->SnapDist.x = 0;
				break;
			}
			case ResizeDirection::top:
			{

				if (fabs(distances_vertical[VerticalDistance::TopToTop]) < distance_threshold)
				{
					this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
					if (fabs(this->SnapDist.y) < mouse_delta_threshold)
					{
						ApplyDeltaPos({ 0, -distances_vertical[VerticalDistance::TopToTop] });
						ApplyDeltaResize({ 0, distances_vertical[VerticalDistance::TopToTop] });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_vertical[VerticalDistance::TopToMiddle]) < distance_threshold)
				{
					this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
					if (fabs(this->SnapDist.y) < mouse_delta_threshold)
					{
						ApplyDeltaPos({ 0, -distances_vertical[VerticalDistance::TopToMiddle] });
						ApplyDeltaResize({ 0, distances_vertical[VerticalDistance::TopToMiddle] });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_vertical[VerticalDistance::TopToBottom]) < distance_threshold)
				{
					this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
					if (fabs(this->SnapDist.y) < mouse_delta_threshold)
					{
						ApplyDeltaPos({ 0, -distances_vertical[VerticalDistance::TopToBottom] });
						ApplyDeltaResize({ 0, distances_vertical[VerticalDistance::TopToBottom] });
						rval = true;
					}
					reset_snap = true;
				}
				else
					this->SnapDist.y = 0;
				break;
			}
			case ResizeDirection::bottom:
			{
				if (fabs(distances_vertical[VerticalDistance::BottomToBottom]) < distance_threshold)
				{
					this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
					if (fabs(this->SnapDist.y) < mouse_delta_threshold)
					{
						ApplyDeltaResize({ 0, -distances_vertical[VerticalDistance::BottomToBottom]});
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_vertical[VerticalDistance::BottomToMiddle]) < distance_threshold)
				{
					this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
					if (fabs(this->SnapDist.y) < mouse_delta_threshold)
					{
						ApplyDeltaResize({ 0, -distances_vertical[VerticalDistance::BottomToMiddle] });
						rval = true;
					}
					reset_snap = true;
				}
				else if (fabs(distances_vertical[VerticalDistance::BottomToTop]) < distance_threshold)
				{
					this->SnapDist.y += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y;
					if (fabs(this->SnapDist.y) < mouse_delta_threshold)
					{
						ApplyDeltaResize({ 0, -distances_vertical[VerticalDistance::BottomToTop] });
						rval = true;
					}
					reset_snap = true;
				}
				else
					this->SnapDist.y = 0;
				break;
			}
		}

		if (reset_snap)
		{
			if (ImGui::GetMouseDragDelta().x < 0 && previous_drag_delta.x > 0)
				this->SnapDist = { 0,0 };
			if (ImGui::GetMouseDragDelta().x > 0 && previous_drag_delta.x < 0)
				this->SnapDist = { 0,0 };
			if (ImGui::GetMouseDragDelta().y < 0 && previous_drag_delta.y > 0)
				this->SnapDist = { 0,0 };
			if (ImGui::GetMouseDragDelta().y > 0 && previous_drag_delta.y < 0)
				this->SnapDist = { 0,0 };
		}

	}
	if (ImGui::GetMouseDragDelta().x != 0 || ImGui::GetMouseDragDelta().y != 0)
		previous_drag_delta = ImGui::GetMouseDragDelta();

	return rval;
}

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
	ImVec2 mouse_delta_br = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_bl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_tr = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };
	ImVec2 mouse_delta_tl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };

	bool is_mouse_hovering_br = fabs(mouse_delta_br.x) < delta_offset_outter && fabs(mouse_delta_br.y) < delta_offset_outter;
	bool is_mouse_hovering_r = fabs(mouse_delta_br.x) < delta_offset_outter && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_tr = fabs(mouse_delta_tr.x) < delta_offset_outter && fabs(mouse_delta_tr.y) < delta_offset_outter;
	bool is_mouse_hovering_l = fabs(mouse_delta_bl.x) < delta_offset_outter && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_b = fabs(mouse_delta_bl.y) < delta_offset_outter && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_t = fabs(mouse_delta_tr.y) < delta_offset_outter && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_bl = fabs(mouse_delta_bl.x) < delta_offset_outter && fabs(mouse_delta_bl.y) < delta_offset_outter;
	bool is_mouse_hovering_tl = fabs(mouse_delta_tl.x) < delta_offset_outter && fabs(mouse_delta_tl.y) < delta_offset_outter;

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

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !igd::active_workspace->is_dragging && resize_direction == ResizeDirection::none)
	{
		//order must be the corners first to override the sides
		if (is_mouse_hovering_br)
			resize_direction = ResizeDirection::bottom_right;
		else if (is_mouse_hovering_tr)
			resize_direction = ResizeDirection::top_right;
		else if (is_mouse_hovering_bl)
			resize_direction = ResizeDirection::bottom_left;
		else if (is_mouse_hovering_tl)
			resize_direction = ResizeDirection::top_left;
		else if (is_mouse_hovering_r)
			resize_direction = ResizeDirection::right;
		else if (is_mouse_hovering_l)
			resize_direction = ResizeDirection::left;
		else if (is_mouse_hovering_b)
			resize_direction = ResizeDirection::bottom;
		else if (is_mouse_hovering_t)
			resize_direction = ResizeDirection::top;
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
				
				ResizeSnap(ResizeDirection::right);
				if (!ResizeSnap(ResizeDirection::top))
				{
					ApplyDeltaResize({ mouse_drag_delta.x,-mouse_drag_delta.y });
					ApplyDeltaPos({ 0, mouse_drag_delta.y });
				}
				else
					ApplyDeltaResize({ mouse_drag_delta.x,0 });
				break;
			}
			case ResizeDirection::top_left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
				ResizeSnap(ResizeDirection::left);
				if (!ResizeSnap(ResizeDirection::top))
				{
					ApplyDeltaResize(mouse_drag_delta * -1);
					ApplyDeltaPos(mouse_drag_delta);
				}
				else
				{
					ApplyDeltaResize({ -mouse_drag_delta.x,0 });
					ApplyDeltaPos({ mouse_drag_delta.x,0 });
				}

				break;
			}
			case ResizeDirection::bottom_left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
				ResizeSnap(ResizeDirection::left);
				if (!ResizeSnap(ResizeDirection::bottom))
				{
					ApplyDeltaResize({ -mouse_drag_delta.x, mouse_drag_delta.y });
					ApplyDeltaPos({ mouse_drag_delta.x, 0 });
				}
				else
				{
					ApplyDeltaResize({ -mouse_drag_delta.x, 0 });
					ApplyDeltaPos({ mouse_drag_delta.x, 0 });
				}
				break;
			}
			case ResizeDirection::left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				if (!ResizeSnap(resize_direction))
				{
					ApplyDeltaResize({ -mouse_drag_delta.x, 0 });
					ApplyDeltaPos({ mouse_drag_delta.x, 0 });
				}
				break;
			}
			case ResizeDirection::bottom_right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
				ResizeSnap(ResizeDirection::right);
				if (!ResizeSnap(ResizeDirection::bottom))
					ApplyDeltaResize(mouse_drag_delta);
				else
					ApplyDeltaResize({ mouse_drag_delta.x, 0 });
				break;
			}
			case ResizeDirection::right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				if (!ResizeSnap(resize_direction))
				{
					ApplyDeltaResize({ mouse_drag_delta.x, 0 });
				}
				break;
			}
			case ResizeDirection::top:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNS;
				if (!ResizeSnap(resize_direction))
				{
					ApplyDeltaResize({ 0, -mouse_drag_delta.y });
					ApplyDeltaPos({ 0,  mouse_drag_delta.y });
				}
				break;
			}
			case ResizeDirection::bottom:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNS;
				if (!ResizeSnap(resize_direction))
				{
					ApplyDeltaResize({ 0, mouse_drag_delta.y });
				}
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

	if (!ImGui::GetIO().KeyCtrl && (this->drop_new_parent || igd::active_workspace->is_dragging || igd::active_workspace->dragging_select))
		return;

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_None) && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && resize_direction == ResizeDirection::none)
		igd::active_workspace->SelectElement(this);

}

void ImGuiElement::DrawSelection()
{
	ImGuiContext& g = *GImGui;
	float rounding = 0;
	if (this->v_type_id == (int)element_type::window)
		return;

	if (this->v_type_id!=(int)element_type::childwindow)
		rounding = g.Style.FrameRounding;
	else 
		rounding = g.Style.ChildRounding;
	ImGui::GetWindowDrawList()->AddRect(item_rect.Min+ImVec2(1,1), item_rect.Max-ImVec2(1,1), ImColor(0.0f, 1.0f, 0.0f, 0.6f), rounding, 0, 2.0f);
}


void ImGuiElement::Interact()
{

	ImGuiContext& g = *GImGui;
	if (igd::active_workspace->interaction_mode == InteractionMode::designer)
	{
		if (g.MouseCursor == ImGuiMouseCursor_Hand || g.MouseCursor == ImGuiMouseCursor_Arrow || g.MouseCursor == ImGuiMouseCursor_TextInput)
			Select();

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
						if (e == igd::active_workspace->basic_workspace_element)
							continue;
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