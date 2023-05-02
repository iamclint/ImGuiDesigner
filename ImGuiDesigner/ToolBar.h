#pragma once
#include "Application.h"
#include <filesystem>
struct widget
{
	std::filesystem::path file;
	std::string name;
	std::string desc;
	std::string icon_name;
	ImTextureID icon;
	widget() : name(""), file(""), desc(""), icon(nullptr), icon_name("") {}
	widget(std::filesystem::path _file, std::string _name, std::string _desc, ImTextureID _icon, std::string _icon_name) : file(_file), name(_name), desc(_desc), icon(_icon), icon_name(_icon_name) {}
	
};

class ToolBar : public Walnut::Layer
{
public:
	template<typename T>
	bool Tool(std::string name, ImVec2 size, bool handle_click=true, std::string override_tooltip="");
	void RenderElements();
	void UpdateWidgets();
	void RenderCustomWidgets();
	virtual void OnUIRender() override;
	ToolBar();
private:
	std::unordered_map<std::filesystem::path, widget> widgets;
};
