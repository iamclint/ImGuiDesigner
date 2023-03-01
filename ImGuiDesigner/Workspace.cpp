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
	: code{}, elements{}, elements_buffer{}, undo_stack{}, redo_stack{}, active_element(nullptr), copied_element(nullptr)
{
	basic_workspace_element = new ImGuiElement();
	basic_workspace_element->v_inherit_all_colors = false;
	basic_workspace_element->v_inherit_all_styles = false;
	if (igd::workspaces.size() == 0)
		id = "Workspace";
	else
		id = "Workspace " + std::to_string(igd::workspaces.size()) + "##" + ImGuiElement::RandomID(10);
	is_open = true;
}

ImGuiElement* WorkSpace::CreateElementFromJson(nlohmann::json& obj, ImGuiElement* parent)
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
			AddNewElement((ImGuiElement*)b);
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
		parent->children.push_back((ImGuiElement*)b);
		return f;
	}
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

	Colors();
	Styles();
	
	ImGui::Begin(id.c_str(), &is_open, ImGuiWindowFlags_NoSavedSettings);
	if (this==igd::active_workspace)
		KeyBinds();
	if (ImGui::IsWindowAppearing())
	{
		this->active_element = nullptr;
		igd::active_workspace = this;
	}
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
		element->Render();
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
	ImGui::End();
	this->basic_workspace_element->PopColorAndStyles();
}

void WorkSpace::AddNewElement(ImGuiElement* ele)
{

	if (this->active_element && this->active_element->v_can_have_children)
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
