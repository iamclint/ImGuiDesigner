#pragma once
#include "Walnut/Application.h"
class ToolBar : public Walnut::Layer
{
public:
	template<typename T>
	bool Tool(std::string name, float width);
	virtual void OnUIRender() override;
};
