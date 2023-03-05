#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"


class Properties : public Walnut::Layer
{
public:
	Properties() : modified(false), is_workspace(false) {}
	virtual void OnUIRender() override;
	virtual void OnUpdate(float f) override;
	void PropertyLabel(const char* lbl);
	void PropertySeparator();
private:
	void getChildParents(ImGuiElement* parent);
	void getAllChildren(ImGuiElement* parent);
	void buildTree(ImGuiElement* parent);
	void General();
	void Colors();
	void Styles();
	bool modified;
	const float item_width = 200;
	bool is_workspace;
};
