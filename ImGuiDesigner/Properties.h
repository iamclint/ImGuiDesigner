#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"


class Properties : public Walnut::Layer
{
public:
	Properties() : modified(false) {}
	virtual void OnUIRender() override;
	virtual void OnUpdate(float f) override;
	void PropertyLabel(const char* lbl);
	void PropertySeparator();
private:
	void getChildParents(ImGuiElement* parent);
	void getAllChildren(ImGuiElement* parent);
	void buildTree(ImGuiElement* parent);
	bool modified;
};
