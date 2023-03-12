#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"

struct ColorPaletteColor
{
	std::string name;
	ImVec4 color;
	ColorPaletteColor() : name(""), color(0, 0, 0, 0) {}
	ColorPaletteColor(const std::string& name, const ImVec4& color) : name(name), color(color) {}
	ColorPaletteColor(const std::string& name, const ImColor& color) : name(name), color(color) {}
};

class Properties : public Walnut::Layer
{
public:
	Properties() : modified(false), is_workspace(false), active_color(nullptr), color_palette(""), color_palette_colors{} {}
	virtual void OnUIRender() override;
	virtual void OnUpdate(float f) override;
	void PropertyLabel(const char* lbl);
	void PropertySeparator();
private:
	void getChildParents(ImGuiElement* parent);
	void getAllChildren(ImGuiElement* parent);
	void buildTree(ImGuiElement* parent);
	void Tree();
	void General();
	void Colors();
	void Styles();
	void Flags();
	bool ColorSelector(ImVec4 color, std::string title);
	void SavePalette();
	void LoadPalette(std::filesystem::path palette_path);
	std::filesystem::path color_palette;
	std::vector<ColorPaletteColor> color_palette_colors;
	bool modified;
	
	ImColor* active_color;
	const float item_width = 200;
	bool is_workspace;
};
