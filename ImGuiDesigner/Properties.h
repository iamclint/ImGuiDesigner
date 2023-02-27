#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"

class Properties : public Walnut::Layer
{
public:
	ImGuiElement* active_element;
	ImGuiElement* copied_element;
	Properties() : active_element(nullptr), copied_element(nullptr), modified(false), load_font(false) {}
	virtual void OnUIRender() override;
	virtual void OnUpdate(float f) override;
	void PropertyLabel(const char* lbl);
	void PropertySeparator();
	std::unordered_map<std::string, ImFont*> LoadedFonts;
private:
	void getChildParents(ImGuiElement* parent);
	void getAllChildren(ImGuiElement* parent);
	void buildTree(ImGuiElement* parent);
	bool modified;
	bool load_font;
	std::filesystem::path font;
};
