#pragma once
#pragma once
#include "Walnut/Application.h"
#include <filesystem>
//#include "ImGuiElement.h"

class ImGuiElement;

struct ElementFont
{
	std::string name;
	std::string map_name;
	std::filesystem::path path;
	int size;
	ImGuiElement* element;
	ImFont* font;
	
	ElementFont() : font(nullptr), element(nullptr), size(20), path(""), name(""), map_name("") {}
	ElementFont(std::filesystem::path path, int size, ImGuiElement* element)
	{
		this->name = path.filename().string();
		this->map_name = path.filename().string() + ":" + std::to_string(size);
		this->path = path;
		this->size = size;
		this->element = element;
	}
};

class Font
{
private:
	ImFont* sample_font;
public:
	bool valid;
	bool hasSample();
	std::filesystem::path _path;
	void draw_sample();
	bool hasUnicodeEncoding(std::filesystem::path filePath);
	Font(std::filesystem::path p);
	Font() { valid = false; };
	
};

class FontManager : public Walnut::Layer
{
public:
	FontManager() : fonts_need_loaded(false), FontsToLoad{}, LoadedFonts{} {}
	std::filesystem::path GetWindowsFontsDirectory();
	virtual void OnUpdate(float f) override;
	virtual void OnUIRender() override;
	ElementFont* GetFont(std::string name, int size);
	void LoadFont(std::filesystem::path path, int size, ImGuiElement* element);
	void UpdateFonts();
	std::filesystem::path FindFont(std::string name);
	std::unordered_map<std::string, Font> AvailableFonts;
	std::unordered_map<std::string, ElementFont> LoadedFonts;
private:
	void UpdateFontsPath(std::filesystem::path p);
	std::vector<ElementFont> FontsToLoad;
	bool fonts_need_loaded;
};