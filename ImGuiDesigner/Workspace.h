#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"
#include <vector>
class WorkSpace : public Walnut::Layer
{
public:
	std::vector<ImGuiElement*> elements;
	std::vector<ImGuiElement*> elements_buffer;
	virtual void OnUIRender() override;
};

namespace igd
{
	extern WorkSpace* work;
}