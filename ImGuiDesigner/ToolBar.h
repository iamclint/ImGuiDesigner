#pragma once
#include "Walnut/Application.h"
class ToolBar : public Walnut::Layer
{
public:
	template<typename T>
	bool Tool(std::string name, float width);
	void RenderElements();
	void RenderCustomWidgets();
	virtual void OnUIRender() override;
};
