#pragma once
#include "imgui.h"
#include <algorithm>
#include <string>
#include <memory>
#include <vector>

enum class property_flags : int
{
	None = 0,
	label = 1 << 0,  
	color_foreground = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
	color_background = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
	color_background_hovered = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
	color_background_active = 1 << 6,   // Resize every window to its content every frame
	border = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
};

inline property_flags operator|(property_flags a, property_flags b)
{
	return static_cast<property_flags>(static_cast<int>(a) | static_cast<int>(b));
}
inline int operator&(property_flags a, property_flags b)
{
	return static_cast<int>(static_cast<int>(a) & static_cast<int>(b));
}
enum class resize_direction : int
{
	none,
	left,
	right,
	top,
	bottom,
	top_left,
	top_right,
	bottom_left,
	bottom_right
};
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
	ImColor v_background_active;
	ImColor v_background_hovered;
	bool v_border;
	bool v_can_have_children;
	ImGuiElement* v_parent;
	std::vector<ImGuiElement*> children;
	property_flags v_property_flags;

	bool did_move;
	bool did_resize;
	
	ImGuiElement() : v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID(10)), v_label("new element"), v_foreground(ImColor(0, 0, 0, 0)), v_background(ImColor(0, 0, 0, 0)), v_parent(nullptr), v_border(0), v_pos(ImVec2(0,0)), is_dragging(false), resize(resize_direction::none), current_drag_delta(0,0), last_size(0, 0), delete_me(false), v_can_have_children(false), change_parent(nullptr), did_resize(false), did_move(false) {}
	ImGuiElement(const char* id, const char* label, ImVec2 size, int flags, ImColor foreground, ImColor background, ImGuiElement* parent, int border) : v_flags(flags), v_size(size), v_id(id), v_label(label), v_foreground(foreground), v_background(background), v_parent(parent), v_border(0), v_pos(ImVec2(0, 0)), is_dragging(false), resize(resize_direction::none), current_drag_delta(0, 0), last_size(0, 0), delete_me(false), v_can_have_children(false), change_parent(nullptr), did_resize(false), did_move(false){}
	void Render();
	void Delete();
	//overrideable functions
	virtual void RenderPropertiesInternal() = 0;
	virtual void RenderHead() = 0;
	virtual void RenderInternal() = 0;
	virtual void RenderFoot() = 0;
	virtual void Clone() = 0;
	virtual void UndoLocal() = 0;
	virtual void RedoLocal() = 0;
	virtual void PushUndoLocal() = 0;
	void Redo();
	void Undo();
	void PushUndo();
	virtual ~ImGuiElement() {};
	//helper functions
	static std::string RandomID(size_t length);
	bool delete_me;
	ImGuiElement* change_parent;
private:
	bool Drag();
	bool Resize();
	void DrawSelection();
	void KeyMove();
	void Select();
	void KeyBinds();

	ImVec2 current_drag_delta;
	bool is_dragging;
	resize_direction resize;
	ImVec2 last_size;
	ImVec2 last_position;
	ImVec2 last_known_cursor;
	ImVec2 mouse_drag_delta;
};

