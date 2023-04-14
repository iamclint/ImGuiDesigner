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
	: code{}, elements_buffer{}, undoStack{}, redoStack{}, selected_elements{},
	copied_elements{}, loading_workspace(false), is_interacting(false), sort_buffer{},
	interaction_mode(InteractionMode::designer), is_dragging(false), drag_select{}, dragging_select(false), hovered_element(nullptr)
{
	basic_workspace_element = (ImGuiElement*)(new igd::Window());
	basic_workspace_element->v_window_bool = &is_open;
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
			igd::dialogs->GenericNotification("Error", "Unknown element type: " + obj["type"].get<std::string>());
			return nullptr;
		}
	}
	catch (nlohmann::json::exception ex)
	{
		std::cout << "Error parsing json: " << ex.what() << std::endl;
		igd::dialogs->GenericNotification("Error parsing json", ex.what());
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

void WorkSpace::SetSingleSelection(ImGuiElement* ele)
{
	this->selected_elements.clear();
	this->selected_elements.push_back(ele);
}
ImGuiElement* WorkSpace::GetSingleSelection()
{
	if (!this->selected_elements.size())
		return nullptr;
	return this->selected_elements[0];
}

bool doRectanglesIntersect(const ImRect& rect1, const ImRect& rect2)
{
	// Ensure that the coordinates are in the correct order
	float left1 = std::min(rect1.Min.x, rect1.Max.x);
	float top1 = std::min(rect1.Min.y, rect1.Max.y);
	float right1 = std::max(rect1.Min.x, rect1.Max.x);
	float bottom1 = std::max(rect1.Min.y, rect1.Max.y);

	float left2 = std::min(rect2.Min.x, rect2.Max.x);
	float top2 = std::min(rect2.Min.y, rect2.Max.y);
	float right2 = std::max(rect2.Min.x, rect2.Max.x);
	float bottom2 = std::max(rect2.Min.y, rect2.Max.y);

	// Check if the rectangles intersect
	return left1 <= right2 && right1 >= left2 && top1 <= bottom2 && bottom1 >= top2;
}

void WorkSpace::SelectRect(ImGuiElement* element)
{
	
	//check if element is in rect
	if (doRectanglesIntersect(this->drag_select,element->item_rect))
	{
		if (std::find(this->selected_elements.begin(), this->selected_elements.end(), element) == this->selected_elements.end())
			this->selected_elements.push_back(element);
	}
	else
	{
		//remove from selected elements
		for (auto it = this->selected_elements.begin(); it != this->selected_elements.end();)
		{
			if (*it == element)
				it = this->selected_elements.erase(it);
			else
				it++;
		}
	}
	if (!element->v_can_have_children)
		return;
	for (auto& e : element->children)
		SelectRect(e);
}
void WorkSpace::DragSelect()
{
	ImGuiContext& g = *GImGui;
	if (!ImGui::IsAnyItemHovered() && (g.MouseCursor == ImGuiMouseCursor_Hand || g.MouseCursor == ImGuiMouseCursor_Arrow || g.MouseCursor == ImGuiMouseCursor_TextInput))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			selected_elements.clear();
			drag_select.Min = ImGui::GetMousePos();
			drag_select.Max = ImGui::GetMousePos();
			dragging_select = true;
		}
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		drag_select.Max = ImGui::GetMousePos();

	}
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		dragging_select = false;

	if (this->dragging_select)
	{
		SelectRect(this->basic_workspace_element);
		ImGui::GetWindowDrawList()->AddRectFilled(drag_select.Min, drag_select.Max, ImColor(0.0f, 1.0f, 1.0f, 0.1f), 0.0f, 0);
		ImGui::GetWindowDrawList()->AddRect(drag_select.Min, drag_select.Max, ImColor(0.0f, 1.0f, 1.0f, 0.3f), 0.0f, 0, 1.0f);
	}
}

void WorkSpace::SelectAll(ImGuiElement* element, int level)
{
	if (level>0)
		selected_elements.push_back(element);
	if (!element->v_can_have_children)
		return;
	for (auto& e : element->children)
	{
		if (igd::settings->bools["select_children"])
			SelectAll(e, level+1);
		else
			selected_elements.push_back(e);
	}
}

void WorkSpace::KeyBinds()
{
	if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
		return;
	



	if (ImGui::GetIO().KeyShift)
		DragSelect();
	
	if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl)
	{
		for (auto& e : igd::active_workspace->selected_elements)
			std::cout << "Copied element: " << e->v_id << std::endl;
		igd::active_workspace->copied_elements.clear();
		igd::active_workspace->copied_elements.insert(igd::active_workspace->copied_elements.begin(), igd::active_workspace->selected_elements.begin(), igd::active_workspace->selected_elements.end());
	}

	if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::GetIO().KeyCtrl)
	{
		if (igd::active_workspace->copied_elements.size()>0)
		{
			for (auto& e : igd::active_workspace->copied_elements)
			{
				std::cout << "Pasting element: " << e->v_id << std::endl;

				if (e->v_can_have_children)
					e->children.push_back(e->Clone());
				else
					igd::active_workspace->elements_buffer.push_back(e->Clone());
			}
		}

	}
	if (ImGui::IsKeyPressed(ImGuiKey_A) && ImGui::GetIO().KeyCtrl)
	{
		ImGuiElement* element = nullptr;
		if (this->selected_elements.size() > 0 && this->selected_elements[0]->v_can_have_children)
			element = this->selected_elements[0];

		selected_elements.clear();
		if (!igd::settings->bools["select_children"] && element)
		{
			SelectAll(element);
			return;
		}

		SelectAll(this->basic_workspace_element);
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Z) && ImGui::GetIO().KeyCtrl)
	{
		if (undoStack.size() > 0)
		{
			redoStack.push_back(undoStack.back());
			if (undoStack.back()->delete_me)
				undoStack.back()->delete_me = false;
			else
				undoStack.back()->Undo();
			undoStack.pop_back();
		}
		std::cout << "Undo stack size: " << undoStack.size() << std::endl;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Y) && ImGui::GetIO().KeyCtrl)
	{
		if (redoStack.size() > 0)
		{
			if (redoStack.back()->delete_me)
				redoStack.back()->delete_me = false;
			else
				redoStack.back()->Redo();
			redoStack.pop_back();
		}
		std::cout << "Redo stack size: " << redoStack.size() << std::endl;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !igd::dialogs->IsShowing())
	{
		std::string msg = "";
		std::cout << "Press delete?" << std::endl;
		if (!this->selected_elements.size())
			return;
		if (this->selected_elements.size()==1)
			msg = "Are you sure you wish to delete " + this->selected_elements.front()->v_id;
		else
			msg = "Are you sure you wish to delete all the selected elements?";

		igd::dialogs->Confirmation("Delete", msg, "", [this](bool conf) {
			if (!conf)
				return;

			for (auto& e : this->selected_elements)
			{
				if (e->children.size() > 0)
				{
					for (auto& child : e->children)
						child->Delete();
				}
				e->Delete();
			}
			this->selected_elements.clear();
			});
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !ImGui::IsAnyItemActive() && !igd::dialogs->IsShowing())
	{
		igd::active_workspace->selected_elements.clear();
	}
}

void WorkSpace::PushUndo(ImGuiElement* ele)
{
	undoStack.push_back(ele);
	/*if (redo_stack.size() > 0)
	{
		undo_stack.push_back(redo_stack.back());
		redo_stack.push_back(ele);
	}
	else
	{
		undo_stack.push_back(ele);
		redo_stack.push_back(ele);
	}*/
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
		igd::push_designer_theme();
		if (ImGui::Begin("Code Generation", 0))
		{
			std::string co = this->code.str();
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.0f, 0.0f, 0.0f, 1.0f });
			ImGui::InputTextMultiline("##asdfasdfsdaf", &co, {ImGui::GetContentRegionAvail().x-5,ImGui::GetContentRegionAvail().y-5}, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopStyleColor();
		}
		ImGui::End();
		igd::pop_designer_theme();
	}
	this->code.str("");
}
//imgui window clicked



void WorkSpace::RenderAdd()
{
	igd::push_designer_theme();
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
				igd::active_workspace->selected_elements.clear();
				igd::add_workspace = true;
			}
		}
	}
	ImGui::End();
	igd::pop_designer_theme();
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
	
	//if (igd::active_workspace == this)
	//{
	//	igd::active_workspace->KeyBinds();
	//}

	if (!igd::active_workspace->selected_elements.size())
		igd::active_workspace->selected_elements.push_back(igd::active_workspace->basic_workspace_element);

	//if (hovered_element)
	//{
	//	ImGui::GetForegroundDrawList()->AddRect(hovered_element->item_rect.Min, hovered_element->item_rect.Max, ImColor(1.0f, 1.0f, 0.0f, 1.0f), 1.f, 0, 2.0f);
	//}

	this->FixParentChildRelationships(nullptr);
	hovered_element = nullptr;
	ImVec2 region_avail = ImGui::GetContentRegionAvail();


	basic_workspace_element->Render(region_avail, 1, this, std::bind(&WorkSpace::KeyBinds, this));
	if (igd::active_workspace->is_dragging)
	{
		basic_workspace_element->RenderDrag();
	}
	if (igd::active_workspace->hovered_element)
	{
		igd::active_workspace->hovered_element->HandleDrop();
	}


	if (elements_buffer.size() > 0)
	{
		for (auto& element : elements_buffer)
		{
			basic_workspace_element->children.push_back(element);
		}
		elements_buffer.clear();
	}
}

void WorkSpace::AddNewElement(ImGuiElement* ele, bool force_base, bool force_selection)
{
	if (this->selected_elements.size() > 1)
	{
		this->selected_elements.resize(1);
	}

	if (this->selected_elements.size() && this->selected_elements.front()->v_can_have_children && !force_base)
	{
		if (!this->selected_elements.front()->v_can_contain_own_type && this->selected_elements.front()->v_type_id == ele->v_type_id)
		{
			if (this->selected_elements.front()->v_parent)
			{
				this->selected_elements.front()->v_parent->children.push_back(ele);
				ele->v_parent = this->selected_elements.front()->v_parent;
			}
			else
			{
				igd::dialogs->GenericNotification("Error", "Cannot add this element to " + this->selected_elements.front()->v_id + " as it is the base element");
			}
		}
		else
		{
			this->selected_elements.front()->children.push_back(ele);
			this->selected_elements.front()->children.back()->v_parent = this->selected_elements.front();
		}
	}
	else if (this->selected_elements.front())
	{
		if (this->selected_elements.front()->v_parent)
		{
			this->selected_elements.front()->v_parent->children.push_back(ele);
			ele->v_parent = this->selected_elements.front()->v_parent;
		}
		else
		{
			this->selected_elements.front()->children.push_back(ele);
			ele->v_parent = this->selected_elements.front()->v_parent;
		}
	}
	else
	{
		ele->v_parent = this->basic_workspace_element;
		this->basic_workspace_element->children.push_back(ele);
	}
	if (force_selection)
		this->selected_elements.front() = ele;

	ele->InitState();
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
		igd::dialogs->GenericNotification("Error parsing json", ex.what());
	}
}

void WorkSpace::load(std::filesystem::path path)
{

	if (!igd::active_workspace)
	{
		igd::dialogs->GenericNotification("Load Error", "No active workspace");
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
				ImGuiElement* parent = igd::active_workspace->CreateElementFromJson(s["obj"], igd::active_workspace->basic_workspace_element);
				parent->v_pos = ImGuiElementVec2({ 0,0, });
				GetAllChildren(s["obj"], parent);
			}
			else
			{
				ImGuiElement* parent = igd::active_workspace->CreateElementFromJson(s, igd::active_workspace->basic_workspace_element);
				parent->v_pos = ImGuiElementVec2({ 0,0, });
				GetAllChildren(s, parent);
			}
			elements++;
		}
		basic_workspace_element->v_window_bool = &is_open;
		is_open = true;
		i.close();
	}
	catch (nlohmann::json::exception& ex)
	{
		igd::dialogs->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
		std::cerr << "parse error at byte " << ex.what() << std::endl;
		i.close();
	}
	catch (nlohmann::json::parse_error& ex)
	{
		igd::dialogs->GenericNotification("Json Error", ex.what(), "", "Ok", []() {});
		std::cerr << "parse error at byte " << ex.byte << std::endl << ex.what() << std::endl;
		i.close();
	}
}