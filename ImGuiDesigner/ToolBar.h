#pragma once
#include "Walnut/Application.h"
#include <filesystem>
struct widget
{
	std::filesystem::path file;
	std::string name;
	std::string desc;
	ImTextureID icon;
	widget() : name(""), file(""), desc(""), icon(nullptr) {}
	widget(std::filesystem::path _file, std::string _name, std::string _desc, ImTextureID _icon) : file(_file), name(_name), desc(_desc), icon(_icon) {}
	
};

class ToolBar : public Walnut::Layer
{
public:
	template<typename T>
	bool Tool(std::string name, ImVec2 size);
	void RenderElements();
	void UpdateWidgets();
	void RenderCustomWidgets();
	virtual void OnUIRender() override;
	ToolBar();
private:
	std::unordered_map<std::filesystem::path, widget> widgets;
};
