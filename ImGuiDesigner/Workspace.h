#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"
#include <vector>
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
	ImGuiElement* copied_element;
	ImGuiElement* active_element;
	InteractionMode interaction_mode;
	void AddNewElement(ImGuiElement* ele, bool force_base = false, bool force_selection = true);
	ImGuiElement* CreateElementFromJson(nlohmann::json& obj, ImGuiElement* parent);
	void load(std::filesystem::path path);
	std::stringstream code;
	void KeyBinds();
	virtual void OnUIRender() override;
	void PushUndo(ImGuiElement* ele);
	void Styles();
	void Colors();
	void Save(std::string file_path);
	void Open(std::string file_path);
	bool FixParentChildRelationships(ImGuiElement* element);
	void RenderCode();
	void RenderAdd();
	WorkSpace();
	~WorkSpace();
	bool is_open;
	bool loading_workspace;
	bool is_interacting;
	ImGuiElement* basic_workspace_element;
	
private:
	void GenerateStaticVars();

};

//store the states of the elements for each workspace
//need to also store a stack for the workspace above to know in which order to undo/redo
template<typename T>
std::unordered_map<WorkSpace*, std::unordered_map<T*, std::vector<T>>> undoMap;

template<typename T>
std::unordered_map<WorkSpace*, std::unordered_map<T*, std::vector<T>>> redoMap;