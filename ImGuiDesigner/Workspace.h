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
	std::stringstream code;
	void KeyBinds();
	virtual void OnUIRender() override;
	void PushUndo(ImGuiElement* ele);
	void Styles();
	void Colors();
	WorkSpace();
	~WorkSpace();
	std::string id;
	bool is_open;

	ImGuiElement* basic_workspace_element;

	

};

