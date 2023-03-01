#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"
#include <vector>
class WorkSpace : public Walnut::Layer
{
public:
	
	std::vector<ImGuiElement*> elements;
	std::vector<ImGuiElement*> elements_buffer;
	std::vector<ImGuiElement*> undo_stack;
	std::vector<ImGuiElement*> redo_stack;
	ImGuiElement* copied_element;
	ImGuiElement* active_element;
	void AddNewElement(ImGuiElement* ele, bool force_base = false);
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
	WorkSpace();
	~WorkSpace();
	std::string id;
	bool is_open;
	bool loading_workspace;
	ImGuiElement* basic_workspace_element;

	

};

