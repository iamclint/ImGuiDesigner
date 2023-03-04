#pragma once
#include "imgui.h"
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include "../json/single_include/nlohmann/json.hpp"
#include "FontManager.h"
#include "imgui_internal.h"
#include "Workspace.h"
struct ElementFont;
class WorkSpace;
enum class property_flags : int
{
	None = 0,
	label = 1 << 0,
	pos = 1 << 1,   // Disable scrollbars (window can still scroll with mouse or programmatically)
	border = 1 << 2,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
	disabled = 1 << 3,

};

static inline const char* ImGuiStyleVar_Strings[] = {
	"ImGuiStyleVar_Alpha",
	"ImGuiStyleVar_DisabledAlpha",
	"ImGuiStyleVar_WindowPadding",
	"ImGuiStyleVar_WindowRounding",
	"ImGuiStyleVar_WindowBorderSize",
	"ImGuiStyleVar_WindowMinSize",
	"ImGuiStyleVar_WindowTitleAlign",
	"ImGuiStyleVar_ChildRounding",
	"ImGuiStyleVar_ChildBorderSize",
	"ImGuiStyleVar_PopupRounding",
	"ImGuiStyleVar_PopupBorderSize",
	"ImGuiStyleVar_FramePadding",
	"ImGuiStyleVar_FrameRounding",
	"ImGuiStyleVar_FrameBorderSize",
	"ImGuiStyleVar_ItemSpacing",
	"ImGuiStyleVar_ItemInnerSpacing",
	"ImGuiStyleVar_IndentSpacing",
	"ImGuiStyleVar_CellPadding",
	"ImGuiStyleVar_ScrollbarSize",
	"ImGuiStyleVar_ScrollbarRounding",
	"ImGuiStyleVar_GrabMinSize",
	"ImGuiStyleVar_GrabRounding",
	"ImGuiStyleVar_TabRounding",
	"ImGuiStyleVar_ButtonTextAlign",
	"ImGuiStyleVar_SelectableTextAlign",
	"ImGuiStyleVar_LayoutAlign"
};

static inline const char* ImGuiColor_Strings[] = {
	"ImGuiCol_Text",
	"ImGuiCol_TextDisabled",
	"ImGuiCol_WindowBg",              // Background of normal windows
	"ImGuiCol_ChildBg",               // Background of child windows
	"ImGuiCol_PopupBg",               // Background of popups", menus", tooltips windows
	"ImGuiCol_Border",
	"ImGuiCol_BorderShadow",
	"ImGuiCol_FrameBg",               // Background of checkbox", radio button", plot", slider", text input
	"ImGuiCol_FrameBgHovered",
	"ImGuiCol_FrameBgActive",
	"ImGuiCol_TitleBg",
	"ImGuiCol_TitleBgActive",
	"ImGuiCol_TitleBgCollapsed",
	"ImGuiCol_MenuBarBg",
	"ImGuiCol_ScrollbarBg",
	"ImGuiCol_ScrollbarGrab",
	"ImGuiCol_ScrollbarGrabHovered",
	"ImGuiCol_ScrollbarGrabActive",
	"ImGuiCol_CheckMark",
	"ImGuiCol_SliderGrab",
	"ImGuiCol_SliderGrabActive",
	"ImGuiCol_Button",
	"ImGuiCol_ButtonHovered",
	"ImGuiCol_ButtonActive",
	"ImGuiCol_Header",                // Header* colors are used for CollapsingHeader", TreeNode", Selectable", MenuItem
	"ImGuiCol_HeaderHovered",
	"ImGuiCol_HeaderActive",
	"ImGuiCol_Separator",
	"ImGuiCol_SeparatorHovered",
	"ImGuiCol_SeparatorActive",
	"ImGuiCol_ResizeGrip",
	"ImGuiCol_ResizeGripHovered",
	"ImGuiCol_ResizeGripActive",
	"ImGuiCol_Tab",
	"ImGuiCol_TabHovered",
	"ImGuiCol_TabActive",
	"ImGuiCol_TabUnfocused",
	"ImGuiCol_TabUnfocusedActive",
	"ImGuiCol_DockingPreview",        // Preview overlay color when about to docking something
	"ImGuiCol_DockingEmptyBg",        // Background color for empty node (e.g. CentralNode with no window docked into it)
	"ImGuiCol_PlotLines",
	"ImGuiCol_PlotLinesHovered",
	"ImGuiCol_PlotHistogram",
	"ImGuiCol_PlotHistogramHovered",
	"ImGuiCol_TableHeaderBg",         // Table header background
	"ImGuiCol_TableBorderStrong",     // Table outer and header borders (prefer using Alpha=1.0 here)
	"ImGuiCol_TableBorderLight",      // Table inner borders (prefer using Alpha=1.0 here)
	"ImGuiCol_TableRowBg",            // Table row background (even rows)
	"ImGuiCol_TableRowBgAlt",         // Table row background (odd rows)
	"ImGuiCol_TextSelectedBg",
	"ImGuiCol_DragDropTarget",
	"ImGuiCol_NavHighlight",          // Gamepad/keyboard: current highlighted item
	"ImGuiCol_NavWindowingHighlight", // Highlight window when using CTRL+TAB
	"ImGuiCol_NavWindowingDimBg",     // Darken/colorize entire screen behind the CTRL+TAB window list", when active
	"ImGuiCol_ModalWindowDimBg"     // Darken/colorize entire screen behind a modal window", when one is active
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

enum class Vec2Type : int
{
	Absolute,
	Relative
};

struct ImGuiElementVec2
{
	ImVec2 value;
	Vec2Type type;
	ImGuiElementVec2(ImVec2 v, Vec2Type t) : value(v), type(t) {}
	ImGuiElementVec2(ImVec2 v) : value(v), type(Vec2Type::Absolute) {}
	ImGuiElementVec2() : value(), type(Vec2Type::Absolute) {}
};

class STS {
	std::ostringstream ss;
public:
	template<typename T>
	inline STS& operator<<(const T& val) {
		ss << val;
		return *this;
	}
	inline operator std::string() const {
		return ss.str();
	}
};

class ImGuiElement
{
//virtual functions
public:
	virtual void RenderPropertiesInternal();
	//return code string for this element
	virtual std::string RenderHead() { return ""; };
	//return code string for this element
	virtual std::string RenderInternal() { return ""; };
	//return code string for this element
	virtual std::string RenderFoot() { return ""; };
	virtual void Clone() {};
	virtual void UndoLocal();
	virtual void RedoLocal();
	virtual void PushUndoLocal();
	virtual nlohmann::json GetJson();
	virtual void FromJSON(nlohmann::json data);
	virtual ~ImGuiElement() {};

//methods
public:
	void Render(ImVec2 ContentRegionAvail, int current_depth, WorkSpace* workspace);
	void Delete();
	void Redo();
	void Undo();
	void PushUndo();
	void SaveAsWidget(std::string name);
	void PushStyleColor(ImGuiCol idx, const ImVec4& col, void* ws = nullptr);
	void PushStyleVar(ImGuiStyleVar idx, float val, void* ws = nullptr);
	void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val, void* ws = nullptr);
	void PopColorAndStyles(void* ws = nullptr);
	void GenerateStylesColorsJson(nlohmann::json& j, std::string type_name);
	void StylesColorsFromJson(nlohmann::json& j);
	void AllStylesAndColors();
	std::string GetIDForVariable();
	nlohmann::json GetJsonWithChildren();
	static std::string RandomID(size_t length);

//properties	
public:
	int v_flags;
	ImGuiElementVec2 v_size;
	ImGuiElementVec2 v_pos;
	std::string v_id;
	std::string v_label;
	std::map<ImGuiCol_, ColorValue> v_colors;
	std::map<ImGuiStyleVar_, StyleVarValue> v_styles;
	ElementFont v_font;
	bool v_border;
	bool v_can_have_children;
	bool v_disabled;
	bool v_inherit_all_colors;
	bool v_inherit_all_styles;
	bool v_sameline;
	int v_render_index;
	bool needs_resort;
	WorkSpace* v_workspace;
	int v_depth;
	ImVec2 ContentRegionAvail;
	std::string ContentRegionString;
	
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
		v_font = other.v_font;
		v_sameline = other.v_sameline;
	}
		
private:
	bool Drag();
	bool Resize();
	void DrawSelection();
	void KeyMove();
	void Select();
	void KeyBinds();
	void ApplyResize(ImVec2 literal_size);
	void ApplyPos(ImVec2 literal_pos);
	bool ChildrenUseRelative();
	std::string GetContentRegionString();
	void AddCode(std::string code, int depth=-1);
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
