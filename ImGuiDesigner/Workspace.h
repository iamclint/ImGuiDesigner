#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"
#include <vector>
class WorkSpace : public Walnut::Layer
{
public:
	std::vector<ImGuiElement*> elements;
	virtual void OnUIRender() override;
};

namespace igd
{
	extern WorkSpace* work;
}