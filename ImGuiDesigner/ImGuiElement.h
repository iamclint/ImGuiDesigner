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
static inline const char* ImGuiStyleVar_Strings[] = {
	"Alpha",
	"DisabledAlpha",
	"WindowPadding",
	"WindowRounding",
	"WindowBorderSize",
	"WindowMinSize",
	"WindowTitleAlign",
	"ChildRounding",
	"ChildBorderSize",
	"PopupRounding",
	"PopupBorderSize",
	"FramePadding",
	"FrameRounding",
	"FrameBorderSize",
	"ItemSpacing",
	"ItemInnerSpacing",
	"IndentSpacing",
	"CellPadding",
	"ScrollbarSize",
	"ScrollbarRounding",
	"GrabMinSize",
	"GrabRounding",
	"TabRounding",
	"ButtonTextAlign",
	"SelectableTextAlign",
	"LayoutAlign"
};
static inline const char* ImGuiColor_Strings[] = {
	"Text",
	"TextDisabled",
	"WindowBg",              // Background of normal windows
	"ChildBg",               // Background of child windows
	"PopupBg",               // Background of popups", menus", tooltips windows
	"Border",
	"BorderShadow",
	"FrameBg",               // Background of checkbox", radio button", plot", slider", text input
	"FrameBgHovered",
	"FrameBgActive",
	"TitleBg",
	"TitleBgActive",
	"TitleBgCollapsed",
	"MenuBarBg",
	"ScrollbarBg",
	"ScrollbarGrab",
	"ScrollbarGrabHovered",
	"ScrollbarGrabActive",
	"CheckMark",
	"SliderGrab",
	"SliderGrabActive",
	"Button",
	"ButtonHovered",
	"ButtonActive",
	"Header",                // Header* colors are used for CollapsingHeader", TreeNode", Selectable", MenuItem
	"HeaderHovered",
	"HeaderActive",
	"Separator",
	"SeparatorHovered",
	"SeparatorActive",
	"ResizeGrip",
	"ResizeGripHovered",
	"ResizeGripActive",
	"Tab",
	"TabHovered",
	"TabActive",
	"TabUnfocused",
	"TabUnfocusedActive",
	"DockingPreview",        // Preview overlay color when about to docking something
	"DockingEmptyBg",        // Background color for empty node (e.g. CentralNode with no window docked into it)
	"PlotLines",
	"PlotLinesHovered",
	"PlotHistogram",
	"PlotHistogramHovered",
	"TableHeaderBg",         // Table header background
	"TableBorderStrong",     // Table outer and header borders (prefer using Alpha=1.0 here)
	"TableBorderLight",      // Table inner borders (prefer using Alpha=1.0 here)
	"TableRowBg",            // Table row background (even rows)
	"TableRowBgAlt",         // Table row background (odd rows)
	"TextSelectedBg",
	"DragDropTarget",
	"NavHighlight",          // Gamepad/keyboard: current highlighted item
	"NavWindowingHighlight", // Highlight window when using CTRL+TAB
	"NavWindowingDimBg",     // Darken/colorize entire screen behind the CTRL+TAB window list", when active
	"ModalWindowDimBg"     // Darken/colorize entire screen behind a modal window", when one is active
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
enum class StyleVarType : int
{
	Float,
	Vec2,
	Vec4
};
union StyleVarValueU
{
	float Float;
	ImVec2 Vec2;
};
struct StyleVarValue
{
	StyleVarType type;
	StyleVarValueU value;
	bool inherit;
	StyleVarValue() : type(StyleVarType::Float), value{}  {}
	StyleVarValue(float f, bool inherit=false) : type(StyleVarType::Float), value{}, inherit(inherit) { value.Float = f; }
	StyleVarValue(ImVec2 v, bool inherit=false) : type(StyleVarType::Vec2), value{}, inherit(inherit) { value.Vec2 = v; }
};
struct ColorValue
{
	ImVec4 value;
	bool inherit;
	ColorValue() : value{}, inherit(false) {}
	ColorValue(ImVec4 v, bool inherit = false) : value(v), inherit(inherit) {}
};


class ImGuiElement
{
//virtual functions
public:
	virtual void RenderPropertiesInternal();
	virtual void RenderHead() {};
	virtual void RenderInternal() {};
	virtual void RenderFoot() {};
	virtual void Clone() {};
	virtual void UndoLocal();
	virtual void RedoLocal();
	virtual void PushUndoLocal();
	virtual nlohmann::json GetJson();
	virtual void FromJSON(nlohmann::json data);
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
	void PopColorAndStyles();
	void GenerateStylesColorsJson(nlohmann::json& j, std::string type_name);
	void StylesColorsFromJson(nlohmann::json& j);
	static std::string RandomID(size_t length);

//properties	
public:
	int v_flags;
	ImVec2 v_size;
	ImVec2 v_pos;
	std::string v_id;
	std::string v_label;
	std::unordered_map<ImGuiCol_, ColorValue> v_colors;
	std::unordered_map<ImGuiStyleVar_, StyleVarValue> v_styles;
	bool v_border;
	bool v_can_have_children;
	bool v_disabled;
	bool v_inherit_all_colors;
	bool v_inherit_all_styles;
	
	ImGuiElement* v_parent;
	std::vector<ImGuiElement*> children;
	property_flags v_property_flags;

	bool did_move;
	bool did_resize;
	bool delete_me;

	ImGuiElement* change_parent;
	
//constructors
public:
	ImGuiElement();
	
	ImGuiElement(const ImGuiElement& other)
	{
		v_flags = other.v_flags;
		v_size = other.v_size;
		v_pos = other.v_pos;
		v_id = other.v_id;
		v_label = other.v_label;
		v_styles = other.v_styles;
		v_colors = other.v_colors;
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
		v_inherit_all_colors = other.v_inherit_all_colors;
		v_inherit_all_styles = other.v_inherit_all_styles;
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

