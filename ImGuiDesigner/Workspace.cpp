#include "Workspace.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <fstream>
#include <filesystem>
WorkSpace::~WorkSpace()
{
	for (auto& ele : elements)
	{
		delete ele;
	}
}

WorkSpace::WorkSpace() 
	: code{}, elements{}, elements_buffer{}, undo_stack{}, redo_stack{}
{
	basic_workspace_element = new ImGuiElement();
	/*this->basic_workspace_element->v_background = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
	this->basic_workspace_element->v_foreground = ImGui::GetStyleColorVec4(ImGuiCol_Text);
	this->basic_workspace_element->v_ImGuiStyleVar_ChildRounding = ImGui::GetStyle().ChildRounding;
	this->basic_workspace_element->v_ImGuiStyleVar_FrameRounding = ImGui::GetStyle().FrameRounding;
	this->basic_workspace_element->v_ImGuiStyleVar_GrabRounding = ImGui::GetStyle().GrabRounding;
	this->basic_workspace_element->v_ImGuiStyleVar_PopupRounding = ImGui::GetStyle().PopupRounding;
	this->basic_workspace_element->v_ImGuiStyleVar_ScrollbarRounding = ImGui::GetStyle().ScrollbarRounding;
	this->basic_workspace_element->v_ImGuiStyleVar_ScrollbarSize = ImGui::GetStyle().ScrollbarSize;*/
	
	if (igd::workspaces.size() == 0)
		id = "Workspace";
	else
		id = "Workspace " + std::to_string(igd::workspaces.size()) + "##" + ImGuiElement::RandomID(10);
	is_open = true;
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
		igd::properties->active_element = nullptr;
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