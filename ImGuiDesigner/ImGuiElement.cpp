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
#include "Dialogs.h"
#include <boost/algorithm/string.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS

void ImGuiElement::ResetInteraction()
{
	did_move = false;
	did_resize = false;
}

void ImGuiElement::Undo()
{
	if (undoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].size() > 1)
	{
		redoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].push_back(*this);
		undoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].pop_back();
		*this = undoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].back();
		ResetInteraction();
	}
}

void ImGuiElement::Redo()
{
	if (redoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].size() > 0)
	{
		*this = redoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].back();
		undoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].push_back(*this);
		redoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].pop_back();
		igd::active_workspace->PushUndo(this);
		ResetInteraction();
	}
}

void ImGuiElement::InitState()
{
	undoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].push_back(*this);
}

void ImGuiElement::PushUndo()
{
	std::cout << "using generic push undo" << std::endl;
	igd::active_workspace->PushUndo(this);
	undoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].push_back(*this);

	//if you make an edit you lose your redo stack
	redoMap<std::remove_reference<decltype(*this)>::type>[igd::active_workspace][this].clear();
}
ImGuiElement::ImGuiElement()
	: v_flags(ImGuiButtonFlags_None), v_size(ImVec2(0, 0)), v_id(RandomID()), v_label("new element"),
	v_parent(nullptr), v_border(0),
	v_pos(ImVec2(0, 0)), resize_direction(ResizeDirection::none), last_size(0, 0),
	delete_me(false), v_can_have_children(false), change_parent(nullptr), did_resize(false), did_move(false),
	v_disabled(false), v_property_flags(property_flags::None), color_pops(0), style_pops(0), v_inherit_all_colors(false), v_inherit_all_styles(false),
	v_font(), v_sameline(false), v_depth(0), ContentRegionAvail(ImVec2(0, 0)), v_workspace(nullptr), v_render_index(0), needs_resort(false), v_requires_open(false), v_is_open(false), v_window_bool(nullptr),
	v_type_id(0), v_can_contain_own_type(true), v_element_filter(0), v_parent_required_id(0), v_auto_select(true), v_path(""),
	v_aspect_ratio(1.0f), is_child_hovered(false), drop_new_parent(false), was_dragging(false), v_tooltip(""), v_icon(nullptr),
	v_is_dragging(false), SnapDist(0.f)
{
	v_property_flags = property_flags::disabled;
}

void ImGuiElement::SetNextWidth()
{
	if (v_size.type == Vec2Type::Absolute && v_size.value.x != 0)
		ImGui::SetNextItemWidth(v_size.value.x);
	else if (v_size.type == Vec2Type::Relative && v_size.value.x != 0)
		ImGui::SetNextItemWidth(ContentRegionAvail.x * (v_size.value.x / 100));
}
ImVec2 ImGuiElement::GetRawSize()
{
	return { item_rect.Max.x - item_rect.Min.x,item_rect.Max.y - item_rect.Min.y };
}
ImVec2 ImGuiElement::GetSize()
{
	ImGuiContext& g = *GImGui;
	if (v_size.type == Vec2Type::Absolute)
		return v_size.value;
	else if (v_size.type == Vec2Type::Relative)
		return { (ContentRegionAvail.x * (v_size.value.x / 100)) - g.Style.FramePadding.x,(ContentRegionAvail.y * (v_size.value.y / 100)) - g.Style.FramePadding.y };
	return { 0,0 };
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
			igd::dialogs->GenericNotification("Json Error", e.what(), "", "Ok", []() {});
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
	this->AddCode(STS() << "ImGui::PushStyleColor(" << ImGuiColor_Strings[idx] << ", ImVec4(" << col.x << ", " << col.y << ", " << col.z << ", " << col.w << "));");
	ImGui::PushStyleColor(idx, col);
	color_pops++;
}
void ImGuiElement::PushStyleVar(ImGuiStyleVar idx, float val, void* ws)
{
	this->AddCode(STS() << "ImGui::PushStyleVar(" << ImGuiStyleVar_Strings[idx] << ", " << val << ");");
	ImGui::PushStyleVar(idx, val);
	style_pops++;
}
void ImGuiElement::PushStyleVar(ImGuiStyleVar idx, const ImVec2& val, void* ws)
{
	this->AddCode(STS() << "ImGui::PushStyleVar(" << ImGuiStyleVar_Strings[idx] << ", ImVec2(" << val.x << ", " << val.y << "));");
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
					igd::dialogs->GenericNotification("Error", "Unable to locate font [" + v_font.name + "] in filesystem. Please check your font manager.");
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
		igd::dialogs->GenericNotification("Json Error", e.what(), "", "Ok", []() {});
	}
}
nlohmann::json ImGuiElement::ColorToJson(ImVec4 col)
{
	return { { "x", col.x }, { "y", col.y }, { "z", col.z }, { "w", col.w } };
}
nlohmann::json ImGuiElement::ColorToJson(ImColor col)
{
	return ColorToJson(col.Value);
}
ImColor ImGuiElement::JsonToColor(nlohmann::json col)
{
	return ImColor((float)col["x"], (float)col["y"], (float)col["z"], (float)col["w"]);
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

void ImGuiElement::SaveAsWidget(std::string name, std::string desc, std::string icon_name)
{
	//if (children.size() == 0)
	//	return;
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
		igd::dialogs->GenericNotification("File already exists", "Please choose a new id for this widget", "", "Ok", []() { std::cout << "Yay" << std::endl; });
		return;
	}
	std::ofstream file;
	file.open("widgets/" + name + ".wgd");
	nlohmann::json main_obj = GetJsonWithChildren();
	main_obj["obj"]["desc"] = desc;
	main_obj["obj"]["name"] = name;
	main_obj["obj"]["icon_name"] = icon_name;
	file << main_obj.dump() << std::endl;
	file.close();
	std::cout << "Saved to widgets/" + name +".igd" << std::endl;
}
ImVec2 get_mouse_location()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();
	ImVec2 window_pos = ImGui::GetWindowPos();
	return ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
}

void ImGuiElement::Delete()
{
	igd::active_workspace->undoStack.push_back(this);
	this->delete_me = true;
	for (auto& e : igd::active_workspace->copied_elements)
	{
		if (e == this)
			e = nullptr;
	}
}

void ImGuiElement::PopColorAndStyles(void* ws)
{
	WorkSpace* w = (WorkSpace*)ws;
	if (!w)
		w = igd::active_workspace;
	if (style_pops > 0)
	{
		ImGui::PopStyleVar(style_pops);
		AddCode(STS() << "ImGui::PopStyleVar(" << color_pops << ");");
	}
	if (color_pops > 0)
	{
		ImGui::PopStyleColor(color_pops);
		AddCode(STS() << "ImGui::PopStyleColor(" << color_pops << ");");
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
	std::string var_str = sp_id.front();
	boost::replace_all(var_str, " ", "_");
	//add v_depth just as a scope indicator
	return var_str + std::to_string(v_depth);
}
std::vector<std::string> ImGuiElement::GetSplitID()
{
	std::vector<std::string> sp_id;
	boost::split(sp_id, v_id, boost::is_any_of("##"));
	return sp_id;
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
	if (code == "")
		return;
	std::vector<std::string> sp_data;
	boost::split(sp_data, code, boost::is_any_of("\n"));
	if (this->v_workspace)
	{
		for (auto& data : sp_data)
			v_workspace->code << GetCodeTabs(depth==-1 ? v_depth : depth) << data << std::endl;
	}
}

ImVec2 ImGuiElement::GetPos()
{
	if (this->v_is_dragging)
	{
		return ImVec2(this->v_pos_dragging.value.x, this->v_pos_dragging.value.y);
	}
	return this->item_rect.Min;
}


void ImGuiElement::Render(ImVec2 _ContentRegionAvail, int current_depth, WorkSpace* ws, std::function<void()> callback)
{

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	is_child_hovered = false;
	v_workspace = ws;
	v_depth = current_depth;
	ContentRegionAvail = _ContentRegionAvail;
	bool script_only = (v_parent && v_parent->v_requires_open && !v_parent->v_is_open);
	if (ContentRegionString == "")
		ContentRegionString = "ContentRegionAvail";

	if (v_pos.value.x != 0 || v_pos.value.y != 0)
	{
		if (v_pos.type == Vec2Type::Absolute)
		{
			this->AddCode(STS() << "ImGui::SetCursorPos(" << igd::script::GetVec2String(v_pos.value) << ");");
			ImGui::SetCursorPos(v_pos.value);
		}
		else
		{
			this->AddCode(STS() << "ImGui::SetCursorPos({" << ContentRegionString << ".x * " << igd::script::GetFloatString(v_pos.value.x / 100.0f) << ", " << ContentRegionString << ".y * " << igd::script::GetFloatString(v_pos.value.y / 100.0f) << "});");
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
	std::string RenderHead = this->RenderHead(script_only);
	if (RenderHead !="")
		this->AddCode(RenderHead);
	if (ContentRegionAvail.x == 0 && ContentRegionAvail.y == 0)
		ContentRegionAvail = ImGui::GetContentRegionAvail();
	ContentRegionAvailSelf = ImGui::GetContentRegionAvail();
	////irritating that we have to do this but imgui uses tab items sort of like buttons
	//if (this->v_type_id == (int)element_type::tabitem)
	//	Interact();

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
		//this->AddCode(" ", current_depth);
		this->AddCode(this->RenderInternal(script_only), current_depth+1);
		this->AddCode("}");

	}
	else
	{
		this->AddCode(this->RenderInternal(script_only));
	}
	if (callback)
		callback();

	if (this->v_can_have_children)
		v_scroll_position = { ImGui::GetScrollX(), ImGui::GetScrollY() };

	if (this->v_type_id != (int)element_type::window)
	{
		this->AddCode(this->RenderFoot(script_only));
		this->last_position = ImVec2(g.LastItemData.Rect.Min.x - window->Pos.x + ImGui::GetScrollX(), g.LastItemData.Rect.Min.y - window->Pos.y + ImGui::GetScrollY());// ImGui::GetCursorPos();
		this->last_size = ImVec2(g.LastItemData.Rect.Max.x - g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.y - g.LastItemData.Rect.Min.y);
		this->item_rect = ImRect({ g.LastItemData.Rect.Min.x,g.LastItemData.Rect.Min.y }, { g.LastItemData.Rect.Max.x,g.LastItemData.Rect.Max.y });
		HandleHover();
	}
	else
	{
		this->item_rect = { ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize() };
		HandleHover();
		this->AddCode(this->RenderFoot(script_only));
	}

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && this->v_parent)
	{
		ImGuiElement* p = this->v_parent;
		int max_iterations = 1000;
		int iters = 0;
		while (p)
		{
			if (p == this || iters >= max_iterations)
				break;
			p->is_child_hovered = true;
			p = p->v_parent;
			iters++;
		}
		if (iters >= max_iterations)
		{
			igd::dialogs->GenericNotification("Error", "There has been an error iterating parent elements (infinite loop)");
		}
	}


	if (!is_child_hovered)
		Interact();

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