#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"
class Properties : public Walnut::Layer
{
public:
	ImGuiElement* active_element;
	ImGuiElement* copied_element;
	Properties() : active_element(nullptr), copied_element(nullptr) {}
	virtual void OnUIRender() override;
};
namespace igd
{
	extern Properties* properties;
}