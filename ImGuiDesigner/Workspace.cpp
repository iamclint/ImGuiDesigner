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
}
void WorkSpace::Styles()
{
	//j["v_ImGuiStyleVar_DisabledAlpha"] = v_ImGuiStyleVar_DisabledAlpha;
	//j["v_ImGuiStyleVar_WindowPadding"] = { v_ImGuiStyleVar_WindowPadding.x,v_ImGuiStyleVar_WindowPadding.y };
	//j["v_ImGuiStyleVar_WindowRounding"] = v_ImGuiStyleVar_WindowRounding;
	//j["v_ImGuiStyleVar_WindowBorderSize"] = v_ImGuiStyleVar_WindowBorderSize;
	//j["v_ImGuiStyleVar_WindowMinSize"] = { v_ImGuiStyleVar_WindowMinSize.x,v_ImGuiStyleVar_WindowMinSize.y };
	//j["v_ImGuiStyleVar_WindowTitleAlign"] = { v_ImGuiStyleVar_WindowTitleAlign.x,v_ImGuiStyleVar_WindowTitleAlign.y };
	//j["v_ImGuiStyleVar_ChildRounding"] = v_ImGuiStyleVar_ChildRounding;
	//j["v_ImGuiStyleVar_ChildBorderSize"] = v_ImGuiStyleVar_ChildBorderSize;
	//j["v_ImGuiStyleVar_PopupRounding"] = v_ImGuiStyleVar_PopupRounding;
	//j["v_ImGuiStyleVar_PopupBorderSize"] = v_ImGuiStyleVar_PopupBorderSize;
	//j["v_ImGuiStyleVar_FramePadding"] = { v_ImGuiStyleVar_FramePadding.x,v_ImGuiStyleVar_FramePadding.y };
	//j["v_ImGuiStyleVar_FrameRounding"] = v_ImGuiStyleVar_FrameRounding;
	//j["v_ImGuiStyleVar_FrameBorderSize"] = v_ImGuiStyleVar_FrameBorderSize;
	//j["v_ImGuiStyleVar_ItemSpacing"] = { v_ImGuiStyleVar_ItemSpacing.x,v_ImGuiStyleVar_ItemSpacing.y };
	//j["v_ImGuiStyleVar_ItemInnerSpacing"] = { v_ImGuiStyleVar_ItemInnerSpacing.x, v_ImGuiStyleVar_ItemInnerSpacing.y };
	//j["v_ImGuiStyleVar_IndentSpacing"] = v_ImGuiStyleVar_IndentSpacing;
	//j["v_ImGuiStyleVar_CellPadding"] = { v_ImGuiStyleVar_CellPadding.x,v_ImGuiStyleVar_CellPadding.y };
	//j["v_ImGuiStyleVar_ScrollbarSize"] = v_ImGuiStyleVar_ScrollbarSize;
	//j["v_ImGuiStyleVar_ScrollbarRounding"] = v_ImGuiStyleVar_ScrollbarRounding;
	//j["v_ImGuiStyleVar_GrabMinSize"] = v_ImGuiStyleVar_GrabMinSize;
	//j["v_ImGuiStyleVar_GrabRounding"] = v_ImGuiStyleVar_GrabRounding;
	//j["v_ImGuiStyleVar_TabRounding"] = v_ImGuiStyleVar_TabRounding;
	//j["v_ImGuiStyleVar_ButtonTextAlign"] = { v_ImGuiStyleVar_ButtonTextAlign.x,v_ImGuiStyleVar_ButtonTextAlign.y };
	//j["v_ImGuiStyleVar_SelectableTextAlign"] = { v_ImGuiStyleVar_SelectableTextAlign.x,v_ImGuiStyleVar_SelectableTextAlign.y };
	//j["v_ImGuiStyleVar_LayoutAlign"] = v_ImGuiStyleVar_LayoutAlign;
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
}