#pragma once
#include "imgui.h"
#include <algorithm>
#include <string>
#include <memory>
class ImGuiElement
{
public:
	int v_flags;
	ImVec2 v_size;
	ImVec2 v_pos;
	std::string v_id;
	std::string v_label;
	ImColor v_foreground;
	ImColor v_background;
	ImGuiElement* parent;
	ImVec2 last_known_cursor;
	int border;
	ImGuiElement() : v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(random_string(10)), v_label("new element"), v_foreground(ImColor(0, 0, 0, 0)), v_background(ImColor(0, 0, 0, 0)), parent(nullptr), border(0), v_pos(ImVec2(0,0)) {}
	ImGuiElement(const char* id, const char* label, ImVec2 size, int flags, ImColor foreground, ImColor background, ImGuiElement* parent, int border) : v_flags(flags), v_size(size), v_id(id), v_label(label), v_foreground(foreground), v_background(background), parent(parent), border(0), v_pos(ImVec2(0, 0)) {}
	virtual ~ImGuiElement() {};
	virtual void Render() = 0;
	std::string random_string(size_t length);
	void Drag(ImVec2 Item_Location);

private:
	ImVec2 current_drag_delta;
	bool is_dragging;
	
};

