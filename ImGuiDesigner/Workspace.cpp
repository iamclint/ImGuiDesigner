#include "Workspace.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "igd_elements.h"
#include <boost/algorithm/string.hpp>
WorkSpace::~WorkSpace()
{
	for (auto& ele : elements)
	{
		delete ele;
	}
}

WorkSpace::WorkSpace()
	: code{}, elements{}, elements_buffer{}, undo_stack{}, redo_stack{}, active_element(nullptr), copied_element(nullptr), loading_workspace(false), is_interacting(false), sort_buffer{}
{
	basic_workspace_element = new ImGuiElement();
	basic_workspace_element->v_inherit_all_colors = true;
	basic_workspace_element->v_inherit_all_styles = true;
	id = "Workspace##" + ImGuiElement::RandomID();

	basic_workspace_element->AllStylesAndColors();
	is_open = true;
}

std::string WorkSpace::GetIDForVariable()
{
	std::vector<std::string> sp_id;
	boost::split(sp_id, this->id, boost::is_any_of("##"));
	std::string content_region_id = sp_id.front();
	boost::replace_all(content_region_id, " ", "_");
	return content_region_id;
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
	main_obj.push_back(basic_workspace_element->GetJsonWithChildren());
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
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && ImGui::GetIO().KeyCtrl)
	{
		std::cout << "Copied element: " << igd::active_workspace->active_element->v_id << std::endl;
		igd::active_workspace->copied_element = igd::active_workspace->active_element;
	}

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && ImGui::GetIO().KeyCtrl)
	{
		if (igd::active_workspace->copied_element)
		{
			std::cout << "Pasting element: " << igd::active_workspace->copied_element->v_id << std::endl;
			if (igd::active_workspace->active_element->v_can_have_children)
				igd::active_workspace->active_element->children.push_back(igd::active_workspace->copied_element->Clone());
			else
				igd::active_workspace->elements_buffer.push_back(igd::active_workspace->copied_element->Clone());
		}

	}
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
			this->basic_workspace_element->PushStyleColor(c.first, c.second.value, this);
	}
}
void WorkSpace::Styles()
{

	for (auto& c : this->basic_workspace_element->v_styles)
	{
		if (c.second.type == StyleVarType::Float)
			this->basic_workspace_element->PushStyleVar(c.first, c.second.value.Float, this);
		else if (c.second.type == StyleVarType::Vec2)
			this->basic_workspace_element->PushStyleVar(c.first, c.second.value.Vec2, this);
	}
}

void WorkSpace::GenerateStaticVars()
{
	this->code << "static bool " << this->GetIDForVariable() << " = true;" << std::endl;
}
void WorkSpace::RenderCode() 
{
	if (this != igd::active_workspace)
		this->code.str("");

	if (this == igd::active_workspace)
	{
		ImGui::SetNextWindowDockID(ImGui::GetID("VulkanAppDockspace"), ImGuiCond_Once);
		if (ImGui::Begin("Code Generation", 0))
		{
			std::string co = this->code.str();
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.0f, 0.0f, 0.0f, 1.0f });
			ImGui::InputTextMultiline("##asdfasdfsdaf", &co, {ImGui::GetContentRegionAvail().x-5,ImGui::GetContentRegionAvail().y-5}, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		ImGui::End();
	}
	this->code.str("");
}
//imgui window clicked



void WorkSpace::RenderAdd()
{
	ImGui::SetNextWindowDockID(ImGui::GetID("VulkanAppDockspace"), ImGuiCond_Always);
	if (ImGui::Begin("+##toolbar_new_workspace", 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
	{
	}
	if (!ImGui::IsMouseDragging(0))
	{
		ImGuiWindow* window = ImGui::FindWindowByName("+##toolbar_new_workspace");
		window->DockOrder = -(short)(igd::workspaces.size() + 1);
		ImGuiTabBar* tab_bar = window->DockNode->TabBar;
		ImGuiTabItem* tab = ImGui::TabBarFindTabByID(tab_bar, window->TabId);
		ImGui::TabBarQueueReorder(tab_bar, tab, 1);
		if (window->DockTabItemStatusFlags & ImGuiItemStatusFlags_HoveredRect)
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				igd::active_workspace->active_element = nullptr;
				igd::add_workspace = true;
			}
		}
	}
	ImGui::End();

}


bool WorkSpace::FixParentChildRelationships(ImGuiElement* element)
{
	std::vector<ImGuiElement*>* element_vector;
	if (element && element->v_can_have_children)
	{
		element_vector = &element->children;
	}
	else if (!element)
	{
		element_vector = &this->elements;
	}
	else
	{
		return false;
	}
	

	for (auto it = element_vector->begin(); it != element_vector->end();)
	{
		ImGuiElement* cElement = (*it);
		if (!cElement)
		{
			++it;
			continue;
		}
		//if it doesn't have a parent and the element vector is not the base vector add it to it
		if (!cElement->v_parent && element_vector!=&this->elements)
		{
			std::cout << "Moving element " << cElement->v_id << " -> workspace " << "Index: " << cElement->v_render_index  << std::endl;
			if (cElement->v_render_index>=this->elements.size())
				this->elements.push_back(cElement);
			else
				this->elements.emplace(this->elements.begin()+cElement->v_render_index, cElement);
			it = element_vector->erase(it);
			return false;
		}
		else if (cElement->v_parent != element) //if the element has a parent and the parent is not the element we are looking at
		{
			std::cout << "Moving element " << cElement->v_id << " -> " << cElement->v_parent->v_id << " Index: " << cElement->v_render_index << std::endl;
			if (cElement->v_render_index >= cElement->v_parent->children.size())
				cElement->v_parent->children.push_back(cElement);
			else
			{
				cElement->v_parent->children.emplace(cElement->v_parent->children.begin() + cElement->v_render_index, cElement);
			}
			it = element_vector->erase(it);
			return false;
		}
		else
		{
			++it;
		}

		if (cElement->v_can_have_children)
		{
			if (!FixParentChildRelationships(cElement))
				return false;
		}
	}
	return true;
}

void WorkSpace::OnUIRender() {
	
	if (!is_open)
	{
		if (igd::workspaces.size()>1) //don't delete the top most work space
			igd::delete_workspace.push_back(this);
		else
			is_open = true;
		return;
	}
	
	ImGui::SetNextWindowDockID(ImGui::GetID("VulkanAppDockspace"), ImGuiCond_Once);
	ImGui::SetNextWindowSize({ 600, 600 }, ImGuiCond_Once);
	this->FixParentChildRelationships(nullptr);
	this->GenerateStaticVars();

	if (!this->basic_workspace_element->v_inherit_all_colors)
		this->Colors();
	if (!this->basic_workspace_element->v_inherit_all_styles)
		this->Styles();

	if (this->basic_workspace_element->v_font.font)
	{
		this->code << "ImGui::PushFont(" << this->basic_workspace_element->v_font.name << ");" << std::endl;
		ImGui::PushFont(this->basic_workspace_element->v_font.font);
	}
	
	this->code << "ImGui::Begin(\"" << this->id << "\", &" << this->GetIDForVariable() << ", " << this->basic_workspace_element->v_flags << ");" << std::endl << "{" << std::endl;
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
		std::cout << "Appearing" << std::endl;
	}
	ImVec2 region_avail = ImGui::GetContentRegionAvail();
	this->code << "\t\tImVec2 ContentRegionAvail = ImGui::GetContentRegionAvail();" << std::endl;

	if (elements_buffer.size() > 0)
	{
		for (auto& element : elements_buffer)
		{
			elements.push_back(element);
		}
		elements_buffer.clear();
	}
	

	for (int r = 0;auto& element : elements)
	{
		if (!element)
			continue;
		if (element->delete_me)
			continue;
		element->v_render_index = r;
		element->Render(region_avail, 1, this);
		r++;
	}
	
	std::string f = code.str();
	this->code << "}" << std::endl << "ImGui::End();" << std::endl;
	ImGui::End(); 
	
	if (this->basic_workspace_element->v_font.font)
	{
		this->code << "ImGui::PopFont();" << std::endl;
		ImGui::PopFont();
	}
	this->basic_workspace_element->PopColorAndStyles(this);

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