#pragma once
#include "imgui.h"
#include <algorithm>
#include <string>
#include <memory>
class ImGuiElement
{
public:
	//properties
	int v_flags;
	ImVec2 v_size;
	ImVec2 v_pos;
	std::string v_id;
	std::string v_label;
	ImColor v_foreground;
	ImColor v_background;
	int border;

	ImGuiElement* parent;

	ImGuiElement() : v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID(10)), v_label("new element"), v_foreground(ImColor(0, 0, 0, 0)), v_background(ImColor(0, 0, 0, 0)), parent(nullptr), border(0), v_pos(ImVec2(0,0)), is_dragging(false), is_resizing(false), current_drag_delta(0,0), last_size(0, 0), delete_me(false) {}
	ImGuiElement(const char* id, const char* label, ImVec2 size, int flags, ImColor foreground, ImColor background, ImGuiElement* parent, int border) : v_flags(flags), v_size(size), v_id(id), v_label(label), v_foreground(foreground), v_background(background), parent(parent), border(0), v_pos(ImVec2(0, 0)), is_dragging(false), is_resizing(false), current_drag_delta(0, 0), last_size(0, 0), delete_me(false) {}
	void Render();
	void Delete();
	//overrideable functions
	virtual void RenderPropertiesInternal() = 0;
	virtual void RenderInternal() = 0;
	virtual void Clone() = 0;
	virtual ~ImGuiElement() {};
	//helper functions
	std::string RandomID(size_t length);
	bool delete_me;
private:
	void Drag();
	void Resize();
	void DrawSelection();
	void KeyMove();
	void Select();
	void KeyBinds();

	ImVec2 current_drag_delta;
	bool is_dragging;
	bool is_resizing;
	ImVec2 last_size;
	ImVec2 last_known_cursor;
};

