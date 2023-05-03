#include "Workspace.h"
#include "Image.h"
#include "imgui_internal.h"
#include "ImGuiElement.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "igd_elements.h"

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
	interaction_mode(InteractionMode::designer), is_dragging(false), drag_select{}, dragging_select(false), hovered_element(nullptr), 
	is_focused(false), SelectedRect{}, basic_workspace_element(nullptr), multi_drag_element(nullptr), last_selection_time(std::chrono::system_clock::now())
{
	last_selection_time = std::chrono::system_clock::now();
	multi_drag_element = (ImGuiElement*)(new igd::ChildWindow());
	basic_workspace_element = (ImGuiElement*)(new igd::Window());
	basic_workspace_element->v_window_bool = &is_open;
	is_open = true;
	auto lang = TextEditor::LanguageDefinition::CPlusPlus();

	// set your own known preprocessor symbols...
	static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };
	// ... and their corresponding values
	static const char* ppvalues[] = {
		"#define NULL ((void*)0)",
		"#define PM_REMOVE (0x0001)",
		"Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ",
		"enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0",
		"enum D3D_FEATURE_LEVEL",
		"enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
		"#define WINAPI __stdcall",
		"#define D3D11_SDK_VERSION (7)",
		" #define assert(expression) (void)(                                                  \n"
		"    (!!(expression)) ||                                                              \n"
		"    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
		" )"
	};

	for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}

	// set your own identifiers
	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
	static const char* idecls[] =
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };
	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}
	editor.SetLanguageDefinition(lang);
	editor.SetShowWhitespaces(false);
	//editor.SetPalette(TextEditor::GetLightPalette());

	// error markers
	//TextEditor::ErrorMarkers markers;
	//markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	//markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	//editor.SetErrorMarkers(markers);

	// "breakpoint" markers
	//TextEditor::Breakpoints bpts;
	//bpts.insert(24);
	//bpts.insert(47);
	//editor.SetBreakpoints(bpts);
}

void WorkSpace::ResetSelectTimeout()
{
	last_selection_time = std::chrono::system_clock::now();
}
void WorkSpace::SelectElement(ImGuiElement* element)
{
	if (element == this->basic_workspace_element || element == this->multi_drag_element)
		return;

	if (this->CanSelect())
	{
		if (!ImGui::GetIO().KeyCtrl)
		{
			this->SetSingleSelection(element);
		}
		else
		{
			this->selected_elements.erase(std::remove(this->selected_elements.begin(), this->selected_elements.end(), this->basic_workspace_element), this->selected_elements.end());
			bool exists = false;
			for (auto& e : this->selected_elements)
			{
				if (e == element)
				{
					exists = true;
					break;
				}
			}
			if (exists)
			{
				this->selected_elements.erase(std::remove(this->selected_elements.begin(), this->selected_elements.end(), element), this->selected_elements.end());
			}
			else
				this->selected_elements.push_back(element);
		}
		this->SelectedRect = this->GetSelectedRect();
		ResetSelectTimeout();
	}
}

bool WorkSpace::CanSelect()
{
	if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_selection_time).count() < 50)
		return false;
	else
		return true;
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
		return igd::active_workspace->basic_workspace_element;
	return this->selected_elements[0];
}



void WorkSpace::SelectRect(ImGuiElement* element)
{
	
	if (element != this->basic_workspace_element)
	{
		this->selected_elements.erase(std::remove(this->selected_elements.begin(), this->selected_elements.end(), this->basic_workspace_element), this->selected_elements.end());
		//check if element is in rect
		if (igd::doRectanglesIntersect(this->drag_select, element->item_rect))
		{
			if (std::find(this->selected_elements.begin(), this->selected_elements.end(), element) == this->selected_elements.end())
			{
				if (element != this->multi_drag_element && !element->delete_me)
				{
					this->selected_elements.push_back(element);
				}
			}
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
	}
	for (auto& e : element->children)
		SelectRect(e);
}
void WorkSpace::DragSelect()
{
	ImGuiContext& g = *GImGui;
	if ((g.MouseCursor == ImGuiMouseCursor_Hand || g.MouseCursor == ImGuiMouseCursor_Arrow || g.MouseCursor == ImGuiMouseCursor_TextInput))
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
void WorkSpace::DeleteElement()
{
	std::string msg;
	if (!this->selected_elements.size())
		return;
	if (this->selected_elements.size() == 1)
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
void WorkSpace::ResetDrag(ImGuiElement* Parent)
{
	this->is_dragging = false;
	Parent->v_is_dragging = false;
	for (auto& e : Parent->children)
		ResetDrag(e);
}

void WorkSpace::HandleInteraction()
{
	ImGuiContext& g = *GImGui;
	if ((g.MouseCursor >= 2 && g.MouseCursor <= 6) || ImGui::GetIO().KeyShift)
	{
		ImGui::SetCursorPos({ 0, 0 });
		ImGui::InvisibleButton("resize window move blocker!", ImGui::GetWindowSize() - ImVec2(10, 10)); //some bs way to block the window from moving when resizing

	}
	if (this->is_dragging)
	{
		this->basic_workspace_element->RenderDrag();
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && this->is_dragging && this->selected_elements.size() > 0)
	{
		if (this->selected_elements.size() > 1) //some witchcrafted drag snapping for multiselected elements
		{
			ImRect total_selected_rect = this->GetSelectedRect();
			ImVec2 total_selected_rect_size = total_selected_rect.GetSize();
			if (total_selected_rect_size.x > 0 && total_selected_rect_size.y > 0)
			{
				this->multi_drag_element->item_rect = total_selected_rect;
				this->multi_drag_element->v_parent = this->selected_elements.front()->v_parent;
				this->multi_drag_element->v_pos.value = total_selected_rect.Min;
				this->multi_drag_element->v_size.value = total_selected_rect_size;
				this->multi_drag_element->v_pos.type = Vec2Type::Absolute;
				this->multi_drag_element->v_size.type = Vec2Type::Absolute;
				this->multi_drag_element->v_pos_dragging = total_selected_rect.Min;
				this->multi_drag_element->DragSnap();
				for (auto& e : this->selected_elements)
				{
					e->v_pos_dragging.value = this->multi_drag_element->v_pos_dragging.value + e->multi_select_offset;
				}
			}
		}
		else
		{
			for (auto& e : this->selected_elements)
				e->DragSnap();

		}
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
	}
	else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && this->is_dragging)
	{
		this->is_dragging = false;
	}
}

void WorkSpace::KeyBinds()
{
	ImGuiContext& g = *GImGui;
	this->is_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !ImGui::IsAnyItemActive() && !igd::dialogs->IsShowing())
	{
		ResetDrag(this->basic_workspace_element);
		igd::active_workspace->selected_elements.clear();
	}

	if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
		return;

	for (auto& e : selected_elements)
	{
		e->KeyMove();
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !igd::dialogs->IsShowing())
	{
		std::string msg = "";
		std::cout << "Press delete?" << std::endl;
		this->DeleteElement();
	}
	
	if (ImGui::GetIO().KeyShift)
		DragSelect();
	
	if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl)
	{
		for (auto& e : igd::active_workspace->selected_elements)
			std::cout << "Copied element: " << e->v_id << std::endl;
		igd::active_workspace->copied_elements.clear();
		igd::active_workspace->copied_elements.insert(igd::active_workspace->copied_elements.begin(), igd::active_workspace->selected_elements.begin(), igd::active_workspace->selected_elements.end());

		if (igd::settings->bools["select_copy_parent"])
		{
			if (igd::active_workspace->GetSingleSelection()->v_parent)
				igd::active_workspace->SetSingleSelection(igd::active_workspace->GetSingleSelection()->v_parent);
		}
	}

	if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::GetIO().KeyCtrl)
	{
		ResetDrag(this->basic_workspace_element);
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

	static std::string co = this->code.str();

	if (this == igd::active_workspace)
	{
		ImGui::SetNextWindowDockID(ImGui::GetID("VulkanAppDockspace"), ImGuiCond_Once);
		igd::push_designer_theme();
		if (ImGui::Begin("Code Generation", 0))
		{
			if (ImGui::IsWindowAppearing())
			{
				co = this->code.str();
				editor.SetText(co);
			}
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.0f, 0.0f, 0.0f, 1.0f });
			editor.Render("TextEditor", { ImGui::GetContentRegionAvail().x - 5,ImGui::GetContentRegionAvail().y - 5 });
			//ImGui::InputTextMultiline("##asdfasdfsdaf", &co, {ImGui::GetContentRegionAvail().x-5,ImGui::GetContentRegionAvail().y-5}, ImGuiInputTextFlags_ReadOnly);
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

ImRect WorkSpace::GetSelectedRect()
{
	ImRect total_selected_rect = { ImVec2(FLT_MAX, FLT_MAX),ImVec2(-FLT_MAX, -FLT_MAX) };
	for (auto& e : igd::active_workspace->selected_elements)
	{
		if (total_selected_rect.Min.x > e->GetPos().x)
			total_selected_rect.Min.x = e->GetPos().x;
		if (total_selected_rect.Min.y > e->GetPos().y)
			total_selected_rect.Min.y = e->GetPos().y;
		if (total_selected_rect.Max.x < e->GetPos().x + e->GetRawSize().x)
			total_selected_rect.Max.x = e->GetPos().x + e->GetRawSize().x;
		if (total_selected_rect.Max.y < e->GetPos().y + e->GetRawSize().y)
			total_selected_rect.Max.y = e->GetPos().y + e->GetRawSize().y;
	}

	for (auto& e : igd::active_workspace->selected_elements)
	{
		e->multi_select_offset = { e->GetPos().x - total_selected_rect.Min.x, e->GetPos().y - total_selected_rect.Min.y };
	}
	return total_selected_rect;
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
		//	if (cElement->v_render_index>=this->basic_workspace_element->children.size())
				this->basic_workspace_element->children.push_back(cElement);
			//else
			//	this->basic_workspace_element->children.emplace(this->basic_workspace_element->children.begin()+cElement->v_render_index, cElement);
			cElement->v_parent = this->basic_workspace_element;
			it = element_vector->erase(it);
			return false;
		}
		else if (cElement->v_parent != element) //if the element has a parent and the parent is not the element we are looking at
		{
			std::cout << "Moving element " << cElement->v_id << " -> " << cElement->v_parent->v_id << " Index: " << cElement->v_render_index << std::endl;
			//if (cElement->v_render_index >= cElement->v_parent->children.size())
				cElement->v_parent->children.push_back(cElement);
			//else
			//{
			//	cElement->v_parent->children.emplace(cElement->v_parent->children.begin() + cElement->v_render_index, cElement);
			//}
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

void WorkSpace::GenerateVariables(ImGuiElement* p)
{
	std::string d = p->GetVariableCode();
	if (d.length() > 0 && !p->delete_me)
	{
		igd::active_workspace->code << p->GetVariableCode() << std::endl;
	}
	for (auto& e : p->children)
		GenerateVariables(e);
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


	if (!igd::active_workspace->selected_elements.size())
		igd::active_workspace->selected_elements.push_back(igd::active_workspace->basic_workspace_element);


	this->FixParentChildRelationships(nullptr);
	hovered_element = nullptr;
	if (this == igd::active_workspace)
	{
		igd::active_workspace->code << "//ImGuiDesigner generated variables" << std::endl;
		GenerateVariables(igd::active_workspace->basic_workspace_element);
		igd::active_workspace->code << std::endl << std::endl;
	}

	basic_workspace_element->Render({0,0}, 0, this, std::bind(&WorkSpace::KeyBinds, this), false);


	if (this == igd::active_workspace)
	{

		if (this->hovered_element)
		{
			this->hovered_element->HandleDrop();
		}
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
				ImGuiElement* parent = igd::active_workspace->CreateElementFromJson(s["obj"], igd::active_workspace->GetSingleSelection()->v_can_have_children ? igd::active_workspace->GetSingleSelection() : igd::active_workspace->GetSingleSelection()->v_parent);
				parent->v_pos = ImGuiElementVec2({ 0,0, });
				GetAllChildren(s["obj"], parent);
			}
			else
			{
				ImGuiElement* parent = igd::active_workspace->CreateElementFromJson(s, igd::active_workspace->GetSingleSelection()->v_can_have_children ? igd::active_workspace->GetSingleSelection() : igd::active_workspace->GetSingleSelection()->v_parent);
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