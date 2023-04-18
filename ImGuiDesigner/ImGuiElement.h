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
#include "ScriptHelpers.h"
#include "Walnut/Image.h"
struct ElementFont;
class WorkSpace;
enum class property_flags : int
{
	None = 0,
	label = 1 << 0,
	pos = 1 << 1,   
	border = 1 << 2,
	disabled = 1 << 3,
	no_id = 1 << 4,
	no_resize = 1 << 5
};

enum class element_type : int
{
	window = 0,
	button = 1 << 0,
	checkbox = 1 << 1,
	childwindow = 1 << 2,
	combo = 1 << 3,
	inputfloat = 1 << 4,
	inputint = 1 << 5,
	inputtext = 1 << 6,
	selectable = 1 << 7,
	separator = 1 << 8,
	sliderfloat = 1 << 9,
	sliderint = 1 << 10,
	tabbar = 1 << 11,
	tabitem = 1 << 12,
	text = 1 << 13,
	texture = 1 << 14
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
enum class ResizeDirection : int
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
	StyleVarValue(float f, bool inherit=true) : type(StyleVarType::Float), value{}, inherit(inherit) { value.Float = f; }
	StyleVarValue(ImVec2 v, bool inherit=true) : type(StyleVarType::Vec2), value{}, inherit(inherit) { value.Vec2 = v; }
};
struct ColorValue
{
	ImVec4 value;
	bool inherit;
	ColorValue() : value{}, inherit(true) {}
	ColorValue(ImVec4 v, bool inherit = true) : value(v), inherit(inherit) {}
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
private:
	std::ostringstream ss;
	std::unique_ptr<std::vector<char>> buffer;
public:
	template<typename T>
	inline STS& operator<<(const T& val) {
		ss << val;
		return *this;
	}
	inline operator std::string() const {
		return ss.str();
	}
	inline operator const char*() const {
		std::string str = ss.str();
		std::copy(str.begin(), str.end(), std::back_inserter(*buffer.get()));
		return buffer.get()->data();
	}
};


class ImGuiElement
{
//virtual functions
public:
	virtual void RenderPropertiesInternal();
	//return code string for this element
	virtual std::string RenderHead(bool script_only=false) { return ""; };
	//return code string for this element
	virtual std::string RenderInternal(bool script_only=false) { return ""; };
	//return code string for this element
	virtual std::string RenderFoot(bool script_only=false) { return ""; };
	virtual ImGuiElement* Clone() { return nullptr; };
	void InitState();
	virtual nlohmann::json GetJson();
	virtual void FromJSON(nlohmann::json data);
	virtual ~ImGuiElement() {};

//methods
public:
	
	void Render(ImVec2 ContentRegionAvail, int current_depth, WorkSpace* workspace, std::function<void()> callback=nullptr);
	void Delete();
	void Redo();
	void Undo();
	void PushUndo();
	void SaveAsWidget(std::string name, std::string desc, std::string icon_name = "");
	void PushStyleColor(ImGuiCol idx, const ImVec4& col, void* ws = nullptr);
	void PushStyleVar(ImGuiStyleVar idx, float val, void* ws = nullptr);
	void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val, void* ws = nullptr);
	void PopColorAndStyles(void* ws = nullptr);
	void GenerateStylesColorsJson(nlohmann::json& j, std::string type_name);
	void StylesColorsFromJson(nlohmann::json& j);
	void SetNextWidth();
	void HandleDrop();
	void RenderDrag();
	void KeyMove();
	ImVec2 GetPos();
	nlohmann::json ColorToJson(ImVec4 col);
	nlohmann::json ColorToJson(ImColor col);
	ImColor JsonToColor(nlohmann::json col);
	ImVec2 GetSize();
	ImVec2 GetRawSize();
	std::string GetIDForVariable();
	std::vector<std::string> GetSplitID();
	nlohmann::json GetJsonWithChildren();
	static std::string RandomID(size_t length=15);
//properties	
public:
	int v_flags;
	ImGuiElementVec2 v_size;
	ImGuiElementVec2 v_pos;
	ImGuiElementVec2 v_pos_dragging;
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
	bool v_requires_open;
	bool v_is_open;
	bool needs_resort;
	bool* v_window_bool;
	int v_parent_required_id;
	int v_type_id;
	int v_element_filter;
	bool v_can_contain_own_type;
	bool v_auto_select;
	float v_aspect_ratio;
	Walnut::Image* v_icon;
	std::string v_tooltip;
	ImVec2 v_scroll_position;
	ImRect item_rect;
	std::string v_path;
	ImVec2 drag_start_pos;

	std::unordered_map<int, std::string> v_custom_flags;
	std::unordered_map<int, bool> v_custom_flag_groups;
	WorkSpace* v_workspace;
	int v_depth;
	ImVec2 ContentRegionAvail;
	ImVec2 ContentRegionAvailSelf;
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
	
private:
	void ResetInteraction();
	bool Drag();
	bool Resize();
	void DrawSelection();
	void Select();
	void KeyBinds();
	void ApplyResize(ImVec2 literal_size);
	void ApplyDeltaResize(ImVec2 delta);
	void ApplyPos(ImVec2 literal_pos);
	void ApplyDeltaPos(ImVec2 delta);
	void ApplyDeltaPosDrag(ImVec2 delta);
	bool ChildrenUseRelative();
	void Interact();
	void HandleHover();
	std::string GetContentRegionString();

	void AddCode(std::string code, int depth=-1);
	bool drop_new_parent;
	bool was_dragging;

	int color_pops;
	int style_pops;
	int undoStackIndex;
	bool is_child_hovered;
	
	ResizeDirection resize_direction;
	//std::unordered_map<ResizeDirection, ImRect> resize_rects;
	ImVec2 last_size;
	ImVec2 last_position;
	ImVec2 last_known_cursor;
	ImVec2 mouse_drag_delta;
};
