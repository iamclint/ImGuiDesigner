#include "ImGuiElement.h"
#include <iostream>
#include "Properties.h"
#include "imgui_internal.h"
#include <Windows.h>
#include "Workspace.h"
#include "ImGuiDesigner.h"
#include <fstream>
#include <filesystem>
#include <random>
#include "Notifications.h"
#include <boost/algorithm/string.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
//get mouse location delta of item
static inline std::unordered_map<ImGuiElement*, std::vector<ImGuiElement>> undo_stack;
static inline std::unordered_map<ImGuiElement*, std::vector<ImGuiElement>> redo_stack;
void ImGuiElement::UndoLocal() {
	if (undo_stack[this].size() > 1)
	{
		redo_stack[this].push_back(*this);
		if (undo_stack[this].size() > 1)
			undo_stack[this].pop_back();

		*this = undo_stack[this].back();
	}
}
void ImGuiElement::RedoLocal()
{
	if (redo_stack[this].size() > 0)
	{
		*this = redo_stack[this].back();
		PushUndo();
		redo_stack[this].pop_back();
	}
}

void ImGuiElement::PushUndoLocal()
{
	//keep an undo stack locally for this type
	undo_stack[this].push_back(*this);
}
ImGuiElement::ImGuiElement()
	: v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID()), v_label("new element"),
	v_parent(nullptr), v_border(0),
	v_pos(ImVec2(0, 0)), is_dragging(false), ResizeDirection(resize_direction::none), current_drag_delta(0, 0), last_size(0, 0),
	delete_me(false), v_can_have_children(false), change_parent(nullptr), did_resize(false), did_move(false),
	v_disabled(false), v_property_flags(property_flags::None), color_pops(0), style_pops(0), v_inherit_all_colors(true), v_inherit_all_styles(true),
	v_font(), v_sameline(false), v_depth(0), ContentRegionAvail(ImVec2(0, 0)), v_workspace(nullptr), v_render_index(0), needs_resort(false)
{
	v_property_flags = property_flags::disabled;
}
void ImGuiElement::RenderPropertiesInternal()
{
	
}
void ImGuiElement::FromJSON(nlohmann::json data)
{
	StylesColorsFromJson(data);
}
nlohmann::json ImGuiElement::GetJson()
{
	nlohmann::json j;
	GenerateStylesColorsJson(j, "main window");
	return j;
}
void GetAllChildren(ImGuiElement* parent,nlohmann::json& pjson)
{
	ImGuiContext& g = *GImGui;
	for (int i =0;auto& e : parent->children)
	{
		if (e->delete_me)
			continue;
		try
		{
			pjson["children"].push_back(e->GetJson());
		}
		catch (nlohmann::json::exception& e)
		{
			std::cout << "exception: " << e.what() << std::endl;
			igd::notifications->GenericNotification("Json Error", e.what(), "", "Ok", []() {});
		}
		if (e->v_can_have_children)
		{
			GetAllChildren(e, pjson["children"].back());
		}
		i++;
	}
}

void ImGuiElement::PushStyleColor(ImGuiCol idx, const ImVec4& col, void* ws)
{
	WorkSpace* w = (WorkSpace*)ws;
	if (!w)
		w = igd::active_workspace;
	w->code << "ImGui::PushStyleColor(" << ImGuiColor_Strings[idx] << ", ImVec4(" << col.x << ", " << col.y << ", " << col.z << ", " << col.w << "));" << std::endl;
	ImGui::PushStyleColor(idx, col);
	color_pops++;
}
void ImGuiElement::PushStyleVar(ImGuiStyleVar idx, float val, void* ws)
{
	WorkSpace* w = (WorkSpace*)ws;
	if (!w)
		w = igd::active_workspace;
	w->code << "ImGui::PushStyleVar(" << ImGuiStyleVar_Strings[idx] << ", " << val << ");" << std::endl;
	ImGui::PushStyleVar(idx, val);
	style_pops++;
}
void ImGuiElement::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val, void* ws)
{
	WorkSpace* w = (WorkSpace*)ws;
	if (!w)
		w = igd::active_workspace;
	w->code << "ImGui::PushStyleVar(" << ImGuiStyleVar_Strings[idx] << ", ImVec2(" << val.x << ", " << val.y << "));" << std::endl;
	ImGui::PushStyleVar(idx, val);
	style_pops++;
}

void ImGuiElement::StylesColorsFromJson(nlohmann::json& j)
{

	try
	{
		if (this->v_parent || igd::active_workspace->loading_workspace)
		{
			v_pos = { ImVec2(j["pos"]["x"],j["pos"]["y"]), j["pos"]["type"] };
		}

		v_id = j["id"].get<std::string>() + "##" + RandomID();
		v_label = j["label"];
		v_size = { ImVec2(j["size"]["x"],j["size"]["y"]), j["size"]["type"] };
		v_disabled = j["disabled"];
		v_flags = j["flags"];
		v_border = j["border"];
		v_inherit_all_colors = j["inherit_all_colors"];
		v_inherit_all_styles = j["inherit_all_styles"];
		v_font.name = j["font"];
		v_font.size = j["font_size"];
		v_sameline = j["sameline"];
		for (auto& c : j["colors"])
		{
			v_colors[c["id"]] = ColorValue(ImVec4(c["value"][0], c["value"][1], c["value"][2], c["value"][3]), c["inherit"]);

		}
		for (auto& c : j["styles"])
		{
			if (c["type"] == StyleVarType::Float)
				v_styles[c["id"]] = StyleVarValue(c["value"], c["inherit"]);
			else if (c["type"] == StyleVarType::Vec2)
				v_styles[c["id"]] = StyleVarValue(ImVec2(c["value"][0], c["value"][1]), c["inherit"]);
		}
		
		if (v_font.name != "")
		{
			ElementFont* f = igd::font_manager->GetFont(v_font.name, v_font.size);
			if (f && f->name != "") //font is already loaded just use it
			{
				std::cout << "Font already loaded!" << std::endl;
				v_font = *f;
			}
			else
			{
				std::filesystem::path font_path = igd::font_manager->FindFont(v_font.name);
				if (font_path.string() == "") //unable to locate font in filesystem
					igd::notifications->GenericNotification("Error", "Unable to locate font [" + v_font.name + "] in filesystem. Please check your font manager.");
				else
				{
					igd::font_manager->LoadFont(font_path, v_font.size, this);
					std::cout << "Loading font! " << font_path << std::endl;
				}
			}
		}
	}
	catch (nlohmann::json::exception& e)
	{
		igd::notifications->GenericNotification("Json Error", e.what(), "", "Ok", []() {});
	}
}

void ImGuiElement::GenerateStylesColorsJson(nlohmann::json& j, std::string type_name)
{
	j["type"] = type_name;
	size_t pound_pos = v_id.find("#");
	if (pound_pos != std::string::npos)
		j["id"] = v_id.substr(0, pound_pos);
	else
		j["id"] = v_id;
	j["label"] = v_label;
	j["size"] = { {"x", v_size.value.x}, {"y", v_size.value.y}, {"type", v_size.type} };
	j["pos"] = { {"x", v_pos.value.x}, {"y", v_pos.value.y}, {"type", v_pos.type} };
	j["disabled"] = v_disabled;
	j["flags"] = v_flags;
	j["border"] = v_border;
	j["inherit_all_colors"] = v_inherit_all_colors;
	j["inherit_all_styles"] = v_inherit_all_styles;
	j["font"] = v_font.name;
	j["font_size"] = v_font.size;
	j["colors"] = nlohmann::json::array();
	j["sameline"] = v_sameline;
	for (auto& c : v_colors)
		j["colors"].push_back({
			{"id", c.first},
			{"value",{ c.second.value.x, c.second.value.y, c.second.value.z, c.second.value.w}},
			{"inherit", c.second.inherit}
			});

	j["styles"] = nlohmann::json::array();
	for (auto& c : v_styles)
	{
		if (c.second.type == StyleVarType::Float)
			j["styles"].push_back({
				{"type", c.second.type},
				{"id", c.first},
				{"value", c.second.value.Float},
				{"inherit", c.second.inherit}
				});
		else if (c.second.type == StyleVarType::Vec2)
			j["styles"].push_back({
				{"type", c.second.type},
				{"id", c.first},
				{"value", {c.second.value.Vec2.x, c.second.value.Vec2.y}},
				{"inherit", c.second.inherit}
				});
	}
}


nlohmann::json ImGuiElement::GetJsonWithChildren()
{
	nlohmann::json main_obj;
	main_obj["obj"] = this->GetJson();

	if (children.size() == 0)
		return main_obj;

	for (auto& e : children)
	{
		if (e->delete_me)
			continue;
		main_obj["obj"]["children"].push_back(e->GetJson());
		if (e->v_can_have_children)
			GetAllChildren(e, main_obj["obj"]["children"].back());
	}
	return main_obj;
}

void ImGuiElement::SaveAsWidget(std::string name)
{
	if (children.size() == 0)
		return;
	//check if folder exists
	if (!std::filesystem::exists("widgets"))
		std::filesystem::create_directory("widgets");

	size_t find_pound = name.find("#");
	if (find_pound != std::string::npos)
		name = name.substr(0, find_pound);
	//check if file exists
	if (std::filesystem::exists("widgets/" + name + ".wgd"))
	{
		//if it does, do something
		igd::notifications->GenericNotification("File already exists", "Please choose a new id for this widget", "", "Ok", []() { std::cout << "Yay" << std::endl; });
		return;
	}
	std::ofstream file;
	file.open("widgets/" + name + ".wgd");
	nlohmann::json main_obj = GetJsonWithChildren();
	file << main_obj.dump() << std::endl;
	file.close();
	std::cout << "Saved to widgets/" + name +".igd" << std::endl;
}


void ImGuiElement::Undo()
{
	UndoLocal();
	did_move = false;
	did_resize = false;

}
void ImGuiElement::Redo()
{
	RedoLocal();
	did_move = false;
	did_resize = false;

}

void ImGuiElement::PushUndo()
{
	//add an undo layer to the undo stack for the workspace itself
	igd::active_workspace->PushUndo(this);
	PushUndoLocal();
}

ImVec2 get_mouse_location()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();
	ImVec2 window_pos = ImGui::GetWindowPos();
	return ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
}

bool ImGuiElement::Drag()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImVec2 Item_Location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(0) && ResizeDirection==resize_direction::none)
	{
		const ImGuiID id = g.CurrentWindow->GetID(v_label.c_str()); //just something
		ImGui::SetActiveID(id, g.CurrentWindow);
		is_dragging = true;
		ImVec2 mouse_location = get_mouse_location();
		current_drag_delta = { mouse_location.x - Item_Location.x - ImGui::GetScrollX(),mouse_location.y - Item_Location.y - ImGui::GetScrollY() };
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && is_dragging)
	{
		const ImGuiID id = g.CurrentWindow->GetID(v_label.c_str()); //just something
		ImGui::SetActiveID(id, g.CurrentWindow);
		did_move = true;
		g.MouseCursor = ImGuiMouseCursor_ResizeAll;
		ImVec2 mouse_location = get_mouse_location();
		ApplyPos({ mouse_location.x - current_drag_delta.x , mouse_location.y - current_drag_delta.y });
		//v_pos = { mouse_location.x - current_drag_delta.x,mouse_location.y - current_drag_delta.y };
	}
	return is_dragging;
}

void ImGuiElement::KeyMove()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x + ImGui::GetScrollX(), g.LastItemData.Rect.Min.y - window->Pos.y + ImGui::GetScrollY());
	float delta_dist = 1.f;
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
	{
		v_pos.value.y = item_location.y - delta_dist;
		v_pos.value.x = item_location.x;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
	{
		v_pos.value.y = item_location.y + delta_dist;
		v_pos.value.x = item_location.x;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
	{
		v_pos.value.x = item_location.x - delta_dist;
		v_pos.value.y = item_location.y;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
	{
		v_pos.value.x = item_location.x + delta_dist;
		v_pos.value.y = item_location.y;
	}
}


void ImGuiElement::KeyBinds()
{

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)) && !ImGui::IsAnyItemActive() && !igd::notifications->IsShowing())
	{
		igd::notifications->Confirmation("Delete", "Are you sure you wish to delete " + this->v_id, "", [this](bool conf) {
			if (!conf)
				return;
			if (this->children.size() > 0)
			{
				for (auto& child : this->children)
					child->Delete();
			}
			this->Delete();
		});
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !ImGui::IsAnyItemActive() && !igd::notifications->IsShowing())
		{
		igd::active_workspace->active_element = nullptr;
	}
}


void ImGuiElement::Delete()
{
	igd::active_workspace->undo_stack.push_back(this);
	this->delete_me = true;
	if (igd::active_workspace->copied_element == this)
		igd::active_workspace->copied_element = nullptr;
	igd::active_workspace->active_element = nullptr;
}

void ImGuiElement::ApplyResize(ImVec2 literal_size)
{
	if (v_size.type == Vec2Type::Absolute)
	{
		v_size.value = literal_size;
	}
	else if (v_size.type == Vec2Type::Relative)
	{
		v_size.value.x = (literal_size.x / ContentRegionAvail.x)*100;
		v_size.value.y = (literal_size.y / ContentRegionAvail.y)*100;
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
		v_pos.value.x = (literal_pos.x / ContentRegionAvail.x)*100;
		v_pos.value.y = (literal_pos.y / ContentRegionAvail.y)*100;
	}
}

bool ImGuiElement::Resize()
{
	if (v_property_flags & property_flags::no_resize)
		return false;
	
	float delta_offset = 5.0f;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiIO& io = g.IO;
	ImVec2 item_location = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x, g.LastItemData.Rect.Min.y - window->Pos.y);
	ImVec2 current_size = ImVec2(g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.y - g.LastItemData.Rect.Min.y);
	ImVec2 mouse_delta_br = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_bl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Max.y - io.MousePos.y };
	ImVec2 mouse_delta_tr = { g.LastItemData.Rect.Max.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };
	ImVec2 mouse_delta_tl = { g.LastItemData.Rect.Min.x - io.MousePos.x,g.LastItemData.Rect.Min.y - io.MousePos.y };
	bool is_mouse_hovering_br = fabs(mouse_delta_br.x) < delta_offset && fabs(mouse_delta_br.y) < delta_offset;
	bool is_mouse_hovering_r = fabs(mouse_delta_br.x) < delta_offset && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_tr = fabs(mouse_delta_tr.x) < delta_offset && fabs(mouse_delta_tr.y) < delta_offset;
	bool is_mouse_hovering_l = fabs(mouse_delta_bl.x) < delta_offset && io.MousePos.y > g.LastItemData.Rect.Min.y && io.MousePos.y < g.LastItemData.Rect.Max.y;
	bool is_mouse_hovering_b = fabs(mouse_delta_bl.y) < delta_offset && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_t = fabs(mouse_delta_tr.y) < delta_offset && io.MousePos.x > g.LastItemData.Rect.Min.x && io.MousePos.x < g.LastItemData.Rect.Max.x;
	bool is_mouse_hovering_bl = fabs(mouse_delta_bl.x) < delta_offset && fabs(mouse_delta_bl.y) < delta_offset;
	bool is_mouse_hovering_tl = fabs(mouse_delta_tl.x) < delta_offset && fabs(mouse_delta_tl.y) < delta_offset;
	
	if (ResizeDirection == resize_direction::none)
	{
		if (is_mouse_hovering_br)
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
		else if (is_mouse_hovering_r)
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
		else if (is_mouse_hovering_tr)
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
		else if (is_mouse_hovering_l)
			g.MouseCursor = ImGuiMouseCursor_ResizeEW;
		else if (is_mouse_hovering_b)
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
		else if (is_mouse_hovering_t)
			g.MouseCursor = ImGuiMouseCursor_ResizeNS;
		else if (is_mouse_hovering_bl)
			g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
		else if (is_mouse_hovering_tl)
			g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
	}

	if (ImGui::IsMouseClicked(0) && !is_dragging && ResizeDirection == resize_direction::none)
	{
		if (is_mouse_hovering_br)
			ResizeDirection = resize_direction::bottom_right;
		else if (is_mouse_hovering_r)
			ResizeDirection = resize_direction::right;
		else if (is_mouse_hovering_tr)
			ResizeDirection = resize_direction::top_right;
		else if (is_mouse_hovering_l)
			ResizeDirection = resize_direction::left;
		else if (is_mouse_hovering_b)
			ResizeDirection = resize_direction::bottom;
		else if (is_mouse_hovering_t)
			ResizeDirection = resize_direction::top;
		else if (is_mouse_hovering_bl)
			ResizeDirection = resize_direction::bottom_left;
		else if (is_mouse_hovering_tl)
			ResizeDirection = resize_direction::top_left;
		
		last_size = current_size;
		last_position = item_location;
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ResizeDirection!=resize_direction::none)
	{
		
		const ImGuiID id = g.CurrentWindow->GetID(v_label.c_str()); //just something
		ImGui::SetActiveID(id, g.CurrentWindow);
		mouse_drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
		did_resize = true;
		switch (ResizeDirection)
		{
			case resize_direction::top_right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
				ApplyResize({ last_size.x + mouse_drag_delta.x,last_size.y - mouse_drag_delta.y });
				ApplyPos({ last_position.x, last_position.y + mouse_drag_delta.y });
				break;
			}
			case resize_direction::top_left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
				ApplyResize({ last_size.x - mouse_drag_delta.x,last_size.y - mouse_drag_delta.y });
				ApplyPos({ last_position.x + mouse_drag_delta.x, last_position.y + mouse_drag_delta.y });
				break;
			}
			case resize_direction::bottom_left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNESW;
				ApplyResize({ last_size.x - mouse_drag_delta.x,last_size.y + mouse_drag_delta.y });
				ApplyPos({ last_position.x + mouse_drag_delta.x, last_position.y });
				break;
			}
			case resize_direction::left:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				ApplyResize({ last_size.x - mouse_drag_delta.x, last_size.y });
				ApplyPos({ last_position.x + mouse_drag_delta.x, last_position.y });
				break;
			}
			case resize_direction::bottom_right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNWSE;
				ApplyResize({ last_size.x + mouse_drag_delta.x,last_size.y + mouse_drag_delta.y });
				break;
			}
			case resize_direction::right:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeEW;
				ApplyResize({ last_size.x + mouse_drag_delta.x,last_size.y });
				break;
			}
			case resize_direction::top:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNS;
				ApplyResize({ last_size.x ,last_size.y - mouse_drag_delta.y });
				ApplyPos({ last_position.x, last_position.y + mouse_drag_delta.y });
				break;
			}
			case resize_direction::bottom:
			{
				g.MouseCursor = ImGuiMouseCursor_ResizeNS;
				ApplyResize({ last_size.x,last_size.y + mouse_drag_delta.y });
				break;
			}
		}
	}
	return ResizeDirection != resize_direction::none;
}

void ImGuiElement::Select()
{
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	ImGuiWindow* window = g.CurrentWindow;
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !is_dragging && ResizeDirection == resize_direction::none)
	{
		const ImGuiID id = g.CurrentWindow->GetID(v_label.c_str()); //just something
		ImGui::SetActiveID(id/*g.LastItemData.ID*/, g.CurrentWindow); //using actual last item id it causes weirdness in children windows
		igd::active_workspace->active_element = this;
	}

}

void ImGuiElement::DrawSelection()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImRect item_location = ImRect({ g.LastItemData.Rect.Min.x - 4,g.LastItemData.Rect.Min.y - 4 }, { g.LastItemData.Rect.Max.x + 4,g.LastItemData.Rect.Max.y + 4 });
	ImGui::GetWindowDrawList()->AddRect(item_location.Min, item_location.Max, ImColor(0.0f, 1.0f, 0.0f, 1.0f), 0.0f, 0, 2.0f);
}

void ImGuiElement::PopColorAndStyles(void* ws)
{
	WorkSpace* w = (WorkSpace*)ws;
	if (!w)
		w = igd::active_workspace;
	if (style_pops > 0)
	{
		ImGui::PopStyleVar(style_pops);
		w->code << "ImGui::PopStyleVar(" << style_pops << ");" << std::endl;
	}
	if (color_pops > 0)
	{
		ImGui::PopStyleColor(color_pops);
		w->code << "ImGui::PopStyleColor(" << color_pops << ");" << std::endl;
	}
	color_pops = 0;
	style_pops = 0;
}

std::string GetCodeTabs(int depth)
{
	std::string tabs = "";
	for (int i = 0; i < depth; i++)
	{
		tabs += "\t\t";
	}
	return tabs;
}

std::string ImGuiElement::GetIDForVariable()
{
	std::vector<std::string> sp_id;
	boost::split(sp_id, v_id, boost::is_any_of("##"));
	std::string content_region_id = sp_id.front();
	boost::replace_all(content_region_id, " ", "_");
	return content_region_id + std::to_string(v_depth);
}

std::string ImGuiElement::GetContentRegionString()
{
	std::vector<std::string> sp_id;
	boost::split(sp_id, v_id, boost::is_any_of("##"));
	std::string content_region_id = sp_id.front();
	boost::replace_all(content_region_id, " ", "_");
	return "ContentRegionAvail_" + content_region_id + std::to_string(v_depth);
}

bool ImGuiElement::ChildrenUseRelative()
{
	for (auto& child : this->children)
	{
		if (child->v_pos.type == Vec2Type::Relative || child->v_size.type == Vec2Type::Relative)
			return true;
	}
	return false;
}


void ImGuiElement::AddCode(std::string code, int depth)
{
	std::vector<std::string> sp_data;
	boost::split(sp_data, code, boost::is_any_of("\n"));
	if (this->v_workspace)
	{
		for (auto& data : sp_data)
			v_workspace->code << GetCodeTabs(depth==-1 ? v_depth : depth) << data << std::endl;
	}
}

bool ImGuiElement::IsFlagGroup(std::pair<int, std::string> current_flag)
{
	for (auto& [flag, str] : v_custom_flags)
	{
		bool any_on = (flag & current_flag.first) != 0;
		bool all_on = (this->v_flags & flag) == flag;
		if (flag == current_flag.first)
			continue;
		if (any_on && all_on)
			return true;
	}
	return false;
}

std::string ImGuiElement::buildFlagString()
{
	std::stringstream ss;
	for (auto& [flag, str] : v_custom_flags)
	{
		bool any_on = (flag & this->v_flags) != 0;
		bool all_on = (this->v_flags & flag) == flag;
		bool is_group_on = (v_custom_flag_groups[flag] && all_on);
		if ((flag & v_flags) && (!IsFlagGroup({ flag,str }) || is_group_on))
			ss << str << " | ";
	}
	if (ss.str().length() > 0)
		return ss.str().substr(0, ss.str().length() - 3);
	else
		return "0";
}
void ImGuiElement::Render(ImVec2 _ContentRegionAvail, int current_depth, WorkSpace* ws)
{
	//v_generate_code = generate_code;
	v_workspace = ws;
	v_depth = current_depth;
	ContentRegionAvail = _ContentRegionAvail;
	if (ContentRegionString == "")
		ContentRegionString = "ContentRegionAvail";
	ImGuiContext& g = *GImGui;
	if (v_pos.value.x != 0 || v_pos.value.y != 0)
	{
		if (v_pos.type == Vec2Type::Absolute)
		{
			this->AddCode(STS() << "ImGui::SetCursorPos({" << igd::fString(v_pos.value.x) << ", " << igd::fString(v_pos.value.y) << "});");
			ImGui::SetCursorPos(v_pos.value);
		}
		else
		{
			this->AddCode(STS() << "ImGui::SetCursorPos({" << ContentRegionString << ".x * " << igd::fString(v_pos.value.x / 100.0f) << ", " << ContentRegionString << ".y * " << igd::fString(v_pos.value.y / 100.0f) << "});");
			ImGui::SetCursorPos({ ContentRegionAvail.x * (v_pos.value.x / 100), ContentRegionAvail.y * (v_pos.value.y / 100) });
		}
	}

	bool need_disable_pop = false;
	color_pops = 0;
	style_pops = 0;
	if (v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
	{
		if ((g.CurrentItemFlags & ImGuiItemFlags_Disabled) == 0)
		{
			this->AddCode("ImGui::BeginDisabled();");
			need_disable_pop = true;
			ImGui::BeginDisabled();
		}
			

	}
	
	if (!this->v_inherit_all_colors)
	{
		for (auto& c : this->v_colors)
		{
			if (c.second.inherit)

				continue;
			this->PushStyleColor(c.first, c.second.value);
		}
	}
	if (!this->v_inherit_all_styles)
	{
		for (auto& c : this->v_styles)
		{
			if (c.second.inherit)
				continue;
			if (c.second.type == StyleVarType::Float)
				this->PushStyleVar(c.first, c.second.value.Float);
			else if (c.second.type == StyleVarType::Vec2)
				this->PushStyleVar(c.first, c.second.value.Vec2);
		}
	}
	
	if (this->v_font.font)
	{
		this->AddCode(STS() << "ImGui::PushFont(" << this->v_font.name << ");");
		ImGui::PushFont(this->v_font.font);
	}

	if (this->v_sameline)
	{
		this->AddCode("ImGui::SameLine();");
		ImGui::SameLine();
	}
	std::string RenderHead = this->RenderHead();
	if (RenderHead !="")
		this->AddCode(RenderHead);

	if (this->children.size() > 0)
	{
		ImVec2 region_avail = ImGui::GetContentRegionAvail();
		std::string content_region_string = GetContentRegionString();
		
		this->AddCode("{");
		
		if (this->ChildrenUseRelative())
			this->AddCode(STS() << "ImVec2 " << content_region_string << " = ImGui::GetContentRegionAvail();", current_depth + 1);

		for (int r =0;auto& child : this->children)
		{
			child->ContentRegionString = content_region_string;
			if (child->delete_me)
				continue;
			child->v_render_index = r;
			child->Render(region_avail, current_depth+1, ws);
			r++;
		}
		this->AddCode("}");

	}
	std::string RenderInternal = this->RenderInternal();
	std::string RenderFoot = this->RenderFoot();;
	if (RenderInternal != "")
		this->AddCode(RenderInternal);
	if (RenderFoot != "")
		this->AddCode(RenderFoot);
	
	if (v_disabled && (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0 && need_disable_pop)
	{
		ImGui::EndDisabled();
		this->AddCode("ImGui::EndDisabled();");
	}
	if (this->v_font.font)
	{
		ImGui::PopFont();
		this->AddCode("ImGui::PopFont();");
	}
	PopColorAndStyles();

	////reset imgui cursorpos so you don't interrupt the flow of other elements when you drag this one
	//if (v_pos.value.x != 0 || v_pos.value.y != 0)
	//{
	//	ImGui::SetCursorPos(last_known_cursor);

	//}
	//else
	//	last_known_cursor = ImGui::GetCursorPos();
	if (igd::active_workspace->interaction_mode == InteractionMode::designer)
	{
		if (igd::active_workspace->active_element == this)
		{
			if (Resize() || Drag())
				igd::active_workspace->is_interacting = true;
			else
				igd::active_workspace->is_interacting = false;
			KeyMove();
			DrawSelection();
			KeyBinds();

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
			ResizeDirection = resize_direction::none;
			if (did_move)
			{
				PushUndo();
				did_move = false;
			}
			is_dragging = false;
		}
	}
}

//random number generator
int GetRandomInt(int min, int max)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution <double> dist(min, max);
	return dist(mt);
}

std::string ImGuiElement::RandomID(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[GetRandomInt(0,max_index)];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}