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
	for (auto& ele : basic_workspace_element->children)
	{
		delete ele;
	}
}

WorkSpace::WorkSpace()
	: code{}, elements_buffer{}, undo_stack{}, redo_stack{}, active_element(nullptr), copied_element(nullptr), loading_workspace(false), is_interacting(false), sort_buffer{}
{
	basic_workspace_element = (ImGuiElement*)(new igd::Window());
	is_open = true;
}

ImGuiElement* WorkSpace::CreateElementFromJson(nlohmann::json& obj, ImGuiElement* parent)
{
	try
	{
		if (igd::element_load_functions[obj["type"]])
			return igd::element_load_functions[obj["type"]](parent, obj);
		else
		{
			igd::notifications->GenericNotification("Error", "Unknown element type: " + obj["type"].get<std::string>());
			return nullptr;
		}
	}
	catch (nlohmann::json::exception ex)
	{
		std::cout << "Error parsing json: " << ex.what() << std::endl;
		igd::notifications->GenericNotification("Error parsing json", ex.what());
	}
	return nullptr;
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
	for (auto& e : basic_workspace_element->children)
	{
		if (e->delete_me)
			continue;
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
		element = this->basic_workspace_element;
		element_vector = &this->basic_workspace_element->children;
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
		if (!cElement->v_parent)
		{
			std::cout << "Moving element " << cElement->v_id << " -> workspace " << "Index: " << cElement->v_render_index  << std::endl;
			if (cElement->v_render_index>=this->basic_workspace_element->children.size())
				this->basic_workspace_element->children.push_back(cElement);
			else
				this->basic_workspace_element->children.emplace(this->basic_workspace_element->children.begin()+cElement->v_render_index, cElement);
			cElement->v_parent = this->basic_workspace_element;
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
	
	if (!igd::active_workspace->active_element)
		igd::active_workspace->active_element = igd::active_workspace->basic_workspace_element;

	this->FixParentChildRelationships(nullptr);
	

	
	ImVec2 region_avail = ImGui::GetContentRegionAvail();
	basic_workspace_element->Render(region_avail, 1, this);
	if (elements_buffer.size() > 0)
	{
		for (auto& element : elements_buffer)
		{
			basic_workspace_element->children.push_back(element);
		}
		elements_buffer.clear();
	}
}

void WorkSpace::AddNewElement(ImGuiElement* ele, bool force_base)
{
	if (this->active_element && this->active_element->v_can_have_children && !force_base)
	{
		this->active_element->children.push_back(ele);
		this->active_element->children.back()->v_parent = this->active_element;
	}
	else if (this->active_element)
	{
		if (this->active_element->v_parent)
		{
			this->active_element->v_parent->children.push_back(ele);
			ele->v_parent = this->active_element->v_parent;
		}
		else
		{
			this->active_element->children.push_back(ele);
			ele->v_parent = this->active_element->v_parent;
		}
	}
	else
	{
		ele->v_parent = this->basic_workspace_element;
		this->basic_workspace_element->children.push_back(ele);
	}
	this->active_element = ele;
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