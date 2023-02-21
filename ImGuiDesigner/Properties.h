#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"

class Properties : public Walnut::Layer
{
public:
	ImGuiElement* active_element;
	ImGuiElement* copied_element;
	Properties() : active_element(nullptr), copied_element(nullptr), modified(false) {}
	virtual void OnUIRender() override;
private:
	void getChildParents(ImGuiElement* parent);
	void getAllChildren(ImGuiElement* parent);
	void buildTree(ImGuiElement* parent);
	bool modified;
};
