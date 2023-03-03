#include "Workspace.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "igd_elements.h"
WorkSpace::~WorkSpace()
{
	for (auto& ele : elements)
	{
		delete ele;
	}
}

WorkSpace::WorkSpace()
	: code{}, elements{}, elements_buffer{}, undo_stack{}, redo_stack{}, active_element(nullptr), copied_element(nullptr), loading_workspace(false), is_interacting(false)
{
	basic_workspace_element = new ImGuiElement();
	basic_workspace_element->v_inherit_all_colors = true;
	basic_workspace_element->v_inherit_all_styles = true;
	if (igd::workspaces.size() == 0)
		id = "Workspace";
	else
		id = "Workspace " + std::to_string(igd::workspaces.size()) + "##" + ImGuiElement::RandomID(10);
	is_open = true;
}

ImGuiElement* WorkSpace::CreateElementFromJson(nlohmann::json& obj, ImGuiElement* parent)
{
	try
	{
		if (obj["type"] == "main window")
		{
			std::cout << "Main Window found" << std::endl;
			igd::active_workspace->basic_workspace_element->FromJSON(obj);
			return igd::active_workspace->basic_workspace_element;
		}
		if (obj["type"] == "child window")
		{
			ImGuiElement* new_parent = nullptr;
			std::cout << "Child Window found" << std::endl;
			igd::ChildWindow* b = new igd::ChildWindow();
			b->FromJSON(obj);
			new_parent = (ImGuiElement*)b;
			if (parent)
				parent->children.push_back((ImGuiElement*)b);
			else
				AddNewElement((ImGuiElement*)b, true);
			new_parent->v_parent = parent;
			return new_parent;
		}
		else if (obj["type"] == "button")
		{
			std::cout << "Adding a button" << std::endl;
			igd::Button* b = new igd::Button();
			ImGuiElement* f = (ImGuiElement*)b;
			f->v_parent = parent;
			b->FromJSON(obj);
			if (!parent)
				igd::active_workspace->AddNewElement((ImGuiElement*)b,true);
			else
				parent->children.push_back((ImGuiElement*)b);
			
			return f;
		}
	}
	catch (nlohmann::json::exception ex)
	{
		std::cout << "Error parsing json: " << ex.what() << std::endl;
		igd::notifications->GenericNotification("Error parsing json", ex.what());
	}
}

void WorkSpace::Open(std::string file_path)
{
	loading_workspace = true;
	load(file_path);
	loading_workspace = false;
}
void WorkSpace::Save(std::string file_path)
{
	std::ofstream file;
	file.open(file_path);
	nlohmann::json main_obj;
	for (auto& e : elements)
	{
		main_obj.push_back(e->GetJsonWithChildren());
	}
	file << main_obj.dump() << std::endl;
	file.close();
	//SaveAsWidget(igd::active_workspace->active_element->v_id);
}

void WorkSpace::KeyBinds()
{
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)) && ImGui::GetIO().KeyCtrl)
	{
		if (undo_stack.size() > 0)
		{
			redo_stack.push_back(undo_stack.back());
			if (undo_stack.back()->delete_me)
				undo_stack.back()->delete_me = false;
			else
				undo_stack.back()->Undo();
			undo_stack.pop_back();
		}
		std::cout << "Undo stack size: " << undo_stack.size() << std::endl;
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Y)) && ImGui::GetIO().KeyCtrl)
	{
		if (redo_stack.size() > 0)
		{
			if (redo_stack.back()->delete_me)
				redo_stack.back()->delete_me = false;
			else
				redo_stack.back()->Redo();
			redo_stack.pop_back();
		}
		std::cout << "Redo stack size: " << redo_stack.size() << std::endl;
	}
}

void WorkSpace::PushUndo(ImGuiElement* ele)
{ 
	undo_stack.push_back(ele);
}

void WorkSpace::Colors()
{
	for (auto& c : this->basic_workspace_element->v_colors)
	{
		if (!c.second.inherit)
			this->basic_workspace_element->PushStyleColor(c.first, c.second.value);
	}
}
void WorkSpace::Styles()
{

	for (auto& c : this->basic_workspace_element->v_styles)
	{
		if (c.second.type == StyleVarType::Float)
			this->basic_workspace_element->PushStyleVar(c.first, c.second.value.Float);
		else if (c.second.type == StyleVarType::Vec2)
			this->basic_workspace_element->PushStyleVar(c.first, c.second.value.Vec2);
	}
}

void WorkSpace::GenerateStaticVars()
{
	igd::active_workspace->code << "static bool " << this->id << " = true;" << std::endl;
}

void WorkSpace::OnUIRender() {
	if (!is_open)
	{
		if (this != igd::workspaces.front()) //don't delete the top most work space
		{
			igd::delete_workspace.push_back(this);
		}
		else
		{
			is_open = true;
		}
		return;
	}
	this->code.str("");

	ImGui::SetNextWindowDockID(ImGui::GetID("VulkanAppDockspace"), ImGuiCond_Once);
	ImGui::SetNextWindowSize({ 600, 600 }, ImGuiCond_Once);

	this->GenerateStaticVars();

	if (!this->basic_workspace_element->v_inherit_all_colors)
		this->Colors();
	if (!this->basic_workspace_element->v_inherit_all_styles)
		this->Styles();

	if (this->basic_workspace_element->v_font.font)
	{
		igd::active_workspace->code << "ImGui::PushFont(" << this->basic_workspace_element->v_font.name << ");" << std::endl;
		ImGui::PushFont(this->basic_workspace_element->v_font.font);
	}
	igd::active_workspace->code << "ImGui::Begin(\"" << this->id << "\", &" << this->id << ", " << this->basic_workspace_element->v_flags << ");" << std::endl << "{" << std::endl;
	int flags = ImGuiWindowFlags_NoSavedSettings;
	if (igd::active_workspace->active_element && this->is_interacting)
		flags |= ImGuiWindowFlags_NoMove;
	
	ImGui::Begin(id.c_str(), &is_open, flags | this->basic_workspace_element->v_flags);
	if (this==igd::active_workspace)
		KeyBinds();
	if (ImGui::IsWindowAppearing())
	{
		this->active_element = nullptr;
		igd::active_workspace = this;
	}
	ImVec2 region_avail = ImGui::GetContentRegionAvail();
	igd::active_workspace->code << "\t\tImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();" << std::endl;

	if (elements_buffer.size() > 0)
	{
		for (auto& element : elements_buffer)
		{
			elements.push_back(element);
		}
		elements_buffer.clear();
	}
	
	for (auto& element : elements)
	{
		if (element->delete_me)
			continue;
		element->Render(region_avail, 1);
	}
	
	//delete from elements if delete_me is true
	for (auto it = elements.begin(); it != elements.end();)
	{
		if ((*it)->v_parent)
		{
			(*it)->v_parent->children.push_back(*it);
			it = elements.erase(it);
		}
		else
		{
			++it;
		}
	}
	std::string f = code.str();
	code << "}" << std::endl << "ImGui::End();" << std::endl;
	ImGui::End(); 
	
	if (this->basic_workspace_element->v_font.font)
	{
		igd::active_workspace->code << "ImGui::PopFont();" << std::endl;
		ImGui::PopFont();
	}
	this->basic_workspace_element->PopColorAndStyles();
	
	ImGui::SetNextWindowDockID(ImGui::GetID("VulkanAppDockspace"), ImGuiCond_Once);
	ImGui::Begin("Code Generation", 0, ImGuiWindowFlags_NoSavedSettings  | ImGuiWindowFlags_NoMove);
		ImGui::TextUnformatted(code.str().c_str());
	ImGui::End();
}

void WorkSpace::AddNewElement(ImGuiElement* ele, bool force_base)
{
	if (this->active_element && this->active_element->v_can_have_children && !force_base)
	{
		this->active_element->children.push_back(ele);
		this->active_element->children.back()->v_parent = this->active_element;
	}
	else
	{
		this->active_element = nullptr;
		this->elements.push_back(ele);
		this->active_element = this->elements.back();
	}
}

void GetAllChildren(nlohmann::json j, ImGuiElement* parent)
{
	try
	{
		ImGuiContext& g = *GImGui;
		for (int i = 0; auto & e : j["children"])
		{
			std::cout << "e: " << e.dump() << std::endl;
			igd::active_workspace->CreateElementFromJson(e, parent);
			if (e["children"].size() > 0)
			{
				GetAllChildren(e, parent->children[i]);
			}
			i++;
		}
	}
	catch (nlohmann::json::exception& ex)
	{
		igd::notifications->GenericNotification("Error parsing json", ex.what());
	}
}

void WorkSpace::load(std::filesystem::path path)
{

	if (!igd::active_workspace)
	{
		igd::notifications->GenericNotification("Load Error", "No active workspace");
		return;
	}
	std::ifstream i(path.string());
	try
	{
		nlohmann::json j;
		j = nlohmann::json::parse(i);
		int elements = 0;
		for (auto& s : j)
		{
			if (s.contains("obj"))
			{
				ImGuiElement* parent = igd::active_workspace->CreateElementFromJson(s["obj"], nullptr);
				GetAllChildren(s["obj"], parent);
			}
			else
			{
				ImGuiElement* parent = igd::active_workspace->CreateElementFromJson(s, nullptr);
				GetAllChildren(s, parent);
			}
			elements++;
		}
		
	}
	catch (nlohmann::json::exception& ex)
	{
		igd::notifications->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
		std::cerr << "parse error at byte " << ex.what() << std::endl;
	}
	catch (nlohmann::json::parse_error& ex)
	{
		igd::notifications->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
		std::cerr << "parse error at byte " << ex.byte << std::endl << ex.what() << std::endl;
	}
}