#pragma once
#include "includes/Application.h"
#include "ImGuiElement.h"
#include <vector>
#include <chrono>
#include "TextEditor.h"
enum class InteractionMode : int
{
	designer,
	user
};


class WorkSpace : public Walnut::Layer
{
public:
	
	//std::vector<ImGuiElement*> elements;
	std::vector<ImGuiElement*> elements_buffer;
	std::vector<ImGuiElement*> undoStack;
	std::vector<ImGuiElement*> redoStack;
	std::vector<ImGuiElement*> sort_buffer;
	std::vector<ImGuiElement*> copied_elements;
	std::vector<ImGuiElement*> selected_elements;
	ImGuiElement* hovered_element;
	InteractionMode interaction_mode;
	ImGuiElement* GetSingleSelection();
	void SetSingleSelection(ImGuiElement* ele);
	void AddNewElement(ImGuiElement* ele, bool force_base = false, bool force_selection = true);
	ImGuiElement* CreateElementFromJson(nlohmann::json& obj, ImGuiElement* parent);
	void load(std::filesystem::path path);
	std::stringstream code;
	void KeyBinds();
	virtual void OnUIRender() override;
	void PushUndo(ImGuiElement* ele);
	void Styles();
	void Colors();
	void DeleteElement();
	void Save(std::string file_path);
	void Open(std::string file_path);
	bool FixParentChildRelationships(ImGuiElement* element);
	void RenderCode();
	void RenderAdd();
	void DragSelect();
	void SelectAll(ImGuiElement* element, int level=0);
	void SelectRect(ImGuiElement* element);
	bool CanSelect();
	void ResetSelectTimeout();
	void SelectElement(ImGuiElement* element);
	void ResetDrag(ImGuiElement* Parent);
	ImRect GetSelectedRect();
	WorkSpace();
	~WorkSpace();
	ImRect drag_select;
	bool is_open;
	bool loading_workspace;
	bool is_interacting;
	bool is_dragging;
	bool dragging_select;
	bool is_focused;
	ImRect SelectedRect;
	ImGuiElement* basic_workspace_element;
	ImGuiElement* multi_drag_element;
	
private:
	void GenerateVariables(ImGuiElement* p);
	std::chrono::system_clock::time_point last_selection_time;
	TextEditor editor;
};

//store the states of the elements for each workspace
//need to also store a stack for the workspace above to know in which order to undo/redo
template<typename T>
std::unordered_map<WorkSpace*, std::unordered_map<T*, std::vector<T>>> undoMap;

template<typename T>
std::unordered_map<WorkSpace*, std::unordered_map<T*, std::vector<T>>> redoMap;