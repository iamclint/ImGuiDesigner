#pragma once
#include "imgui.h"
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
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
	bool v_border;
	bool v_can_have_children;
	ImGuiElement* v_parent;
	std::vector<ImGuiElement*> children;
	ImGuiElement() : v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID(10)), v_label("new element"), v_foreground(ImColor(0, 0, 0, 0)), v_background(ImColor(0, 0, 0, 0)), v_parent(nullptr), v_border(0), v_pos(ImVec2(0,0)), is_dragging(false), is_resizing(false), current_drag_delta(0,0), last_size(0, 0), delete_me(false), v_can_have_children(false), change_parent(nullptr) {}
	ImGuiElement(const char* id, const char* label, ImVec2 size, int flags, ImColor foreground, ImColor background, ImGuiElement* parent, int border) : v_flags(flags), v_size(size), v_id(id), v_label(label), v_foreground(foreground), v_background(background), v_parent(parent), v_border(0), v_pos(ImVec2(0, 0)), is_dragging(false), is_resizing(false), current_drag_delta(0, 0), last_size(0, 0), delete_me(false), v_can_have_children(false), change_parent(nullptr){}
	void Render();
	void Delete();
	//overrideable functions
	virtual void RenderPropertiesInternal() = 0;
	virtual void RenderHead() = 0;
	virtual void RenderInternal() = 0;
	virtual void RenderFoot() = 0;
	virtual void Clone() = 0;

	virtual ~ImGuiElement() {};
	//helper functions
	std::string RandomID(size_t length);
	bool delete_me;
	ImGuiElement* change_parent;
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

