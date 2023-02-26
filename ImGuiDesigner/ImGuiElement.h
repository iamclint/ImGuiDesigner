#pragma once
#include "imgui.h"
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include "../json/single_include/nlohmann/json.hpp"

enum class property_flags : int
{
	None = 0,
	label = 1 << 0,
	color_foreground = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
	color_background = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
	color_background_hovered = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
	color_background_active = 1 << 6,   // Resize every window to its content every frame
	border = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
	disabled = 1 << 8,

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
//virtual functions
public:
	virtual void RenderPropertiesInternal() = 0;
	virtual void RenderHead() = 0;
	virtual void RenderInternal() = 0;
	virtual void RenderFoot() = 0;
	virtual void Clone() = 0;
	virtual void UndoLocal() = 0;
	virtual void RedoLocal() = 0;
	virtual void PushUndoLocal() = 0;
	virtual nlohmann::json GetJson() = 0;
	virtual void FromJSON(nlohmann::json data) = 0;
	virtual ~ImGuiElement() {};
	
//methods
public:
	void Render();
	void Delete();
	void Redo();
	void Undo();
	void PushUndo();
	void SaveAsWidget(std::string name);
	void PushStyleColor(ImGuiCol idx, const ImVec4& col);
	void PushStyleVar(ImGuiStyleVar idx, float val);
	void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);
	
	static std::string RandomID(size_t length);

//properties	
public:
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
	bool v_disabled;
	

	//styles
	float v_ImGuiStyleVar_Alpha;                // float     Alpha
	float v_ImGuiStyleVar_DisabledAlpha;       // float     DisabledAlpha
	ImVec2 v_ImGuiStyleVar_WindowPadding;       // ImVec2    WindowPadding
	float v_ImGuiStyleVar_WindowRounding;      // float     WindowRounding
	float v_ImGuiStyleVar_WindowBorderSize;    // float     WindowBorderSize
	ImVec2 v_ImGuiStyleVar_WindowMinSize;       // ImVec2    WindowMinSize
	ImVec2 v_ImGuiStyleVar_WindowTitleAlign;    // ImVec2    WindowTitleAlign
	float v_ImGuiStyleVar_ChildRounding;       // float     ChildRounding
	float v_ImGuiStyleVar_ChildBorderSize;     // float     ChildBorderSize
	float v_ImGuiStyleVar_PopupRounding;       // float     PopupRounding
	float v_ImGuiStyleVar_PopupBorderSize;     // float     PopupBorderSize
	ImVec2 v_ImGuiStyleVar_FramePadding;        // ImVec2    FramePadding
	float v_ImGuiStyleVar_FrameRounding;       // float     FrameRounding
	float v_ImGuiStyleVar_FrameBorderSize;     // float     FrameBorderSize
	ImVec2 v_ImGuiStyleVar_ItemSpacing;         // ImVec2    ItemSpacing
	ImVec2 v_ImGuiStyleVar_ItemInnerSpacing;    // ImVec2    ItemInnerSpacing
	float v_ImGuiStyleVar_IndentSpacing;       // float     IndentSpacing
	ImVec2 v_ImGuiStyleVar_CellPadding;         // ImVec2    CellPadding
	float v_ImGuiStyleVar_ScrollbarSize;       // float     ScrollbarSize
	float v_ImGuiStyleVar_ScrollbarRounding;   // float     ScrollbarRounding
	float v_ImGuiStyleVar_GrabMinSize;         // float     GrabMinSize
	float v_ImGuiStyleVar_GrabRounding;        // float     GrabRounding
	float v_ImGuiStyleVar_TabRounding;         // float     TabRounding
	ImVec2 v_ImGuiStyleVar_ButtonTextAlign;     // ImVec2    ButtonTextAlign
	ImVec2 v_ImGuiStyleVar_SelectableTextAlign; // ImVec2    SelectableTextAlign
	float v_ImGuiStyleVar_LayoutAlign;         // float     LayoutAlign


	ImGuiElement* v_parent;
	std::vector<ImGuiElement*> children;
	property_flags v_property_flags;

	bool did_move;
	bool did_resize;
	bool delete_me;

	ImGuiElement* change_parent;
	
//constructors
public:
	ImGuiElement()
		: v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID(10)), v_label("new element"),
		v_foreground(ImColor(0, 0, 0, 0)), v_background(ImColor(0, 0, 0, 0)), v_parent(nullptr), v_border(0),
		v_pos(ImVec2(0, 0)), is_dragging(false), resize(resize_direction::none), current_drag_delta(0, 0), last_size(0, 0),
		delete_me(false), v_can_have_children(false), change_parent(nullptr), did_resize(false), did_move(false),
		v_disabled(false), v_property_flags(property_flags::None), color_pops(0), style_pops(0),v_ImGuiStyleVar_Alpha(0),
		v_ImGuiStyleVar_DisabledAlpha(0), v_ImGuiStyleVar_WindowPadding(0, 0), v_ImGuiStyleVar_WindowRounding(0),
		v_ImGuiStyleVar_WindowBorderSize(0), v_ImGuiStyleVar_WindowMinSize(0, 0), v_ImGuiStyleVar_WindowTitleAlign(0, 0),
		v_ImGuiStyleVar_ChildRounding(0), v_ImGuiStyleVar_ChildBorderSize(0), v_ImGuiStyleVar_PopupRounding(0),
		v_ImGuiStyleVar_PopupBorderSize(0), v_ImGuiStyleVar_FramePadding(0, 0), v_ImGuiStyleVar_FrameRounding(0),
		v_ImGuiStyleVar_FrameBorderSize(0), v_ImGuiStyleVar_ItemSpacing(0, 0), v_ImGuiStyleVar_ItemInnerSpacing(0, 0),
		v_ImGuiStyleVar_IndentSpacing(0), v_ImGuiStyleVar_CellPadding(0, 0), v_ImGuiStyleVar_ScrollbarSize(0),
		v_ImGuiStyleVar_ScrollbarRounding(0), v_ImGuiStyleVar_GrabMinSize(0), v_ImGuiStyleVar_GrabRounding(0),
		v_ImGuiStyleVar_TabRounding(0), v_ImGuiStyleVar_ButtonTextAlign(0, 0), v_ImGuiStyleVar_SelectableTextAlign(0, 0),
		v_ImGuiStyleVar_LayoutAlign(0)
		{}
	
	ImGuiElement(const ImGuiElement& other)
	{
		v_flags = other.v_flags;
		v_size = other.v_size;
		v_pos = other.v_pos;
		v_id = other.v_id;
		v_label = other.v_label;
		v_foreground = other.v_foreground;
		v_background = other.v_background;
		v_background_active = other.v_background_active;
		v_background_hovered = other.v_background_hovered;
		v_border = other.v_border;
		v_can_have_children = other.v_can_have_children;
		v_disabled = other.v_disabled;
		v_parent = other.v_parent;
		v_property_flags = other.v_property_flags;
		is_dragging = other.is_dragging;
		resize = other.resize;
		current_drag_delta = other.current_drag_delta;
		last_size = other.last_size;
		delete_me = other.delete_me;
		change_parent = other.change_parent;
		did_resize = other.did_resize;
		did_move = other.did_move;
		color_pops = other.color_pops;
		style_pops = other.style_pops;
	}
		
private:
	bool Drag();
	bool Resize();
	void DrawSelection();
	void KeyMove();
	void Select();
	void KeyBinds();
	
	int color_pops;
	int style_pops;
	ImVec2 current_drag_delta;
	bool is_dragging;
	resize_direction resize;
	ImVec2 last_size;
	ImVec2 last_position;
	ImVec2 last_known_cursor;
	ImVec2 mouse_drag_delta;
};

