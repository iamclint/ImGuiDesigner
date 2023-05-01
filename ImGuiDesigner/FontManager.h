#pragma once
#pragma once
#include "includes/Application.h"
#include <filesystem>
//#include "ImGuiElement.h"

class ImGuiElement;

struct ElementFont
{
	void* font_data;
	int font_data_size;
	std::string name;
	std::string map_name;
	std::filesystem::path path;
	int size;
	ImGuiElement* element;
	ImFont* font;
	
	ElementFont() : font(nullptr), element(nullptr), size(20), path(""), name(""), map_name(""), font_data(nullptr), font_data_size(0) {}
	ElementFont(void* font_data, int font_data_size, std::string name, int size, ImGuiElement* element)
	{
		this->name = name;
		this->map_name = name + ":" + std::to_string(size);
		this->path = "";
		this->font_data = font_data;
		this->font_data_size = font_data_size;
		this->size = size;
		this->element = element;
	}
	ElementFont(std::filesystem::path path, int size, ImGuiElement* element)
	{
		this->name = path.filename().string();
		this->map_name = path.filename().string() + ":" + std::to_string(size);
		this->path = path;
		this->size = size;
		this->element = element;
		this->font_data = nullptr;
		this->font_data_size = 0;
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
	bool operator < (const Font& str) const
	{
		return (str._path.stem().string() < str._path.stem().string());
	}
	
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
	void LoadFont(void* font_data, int font_data_size, std::string name, int size, ImGuiElement* element);
	void UpdateFonts();
	std::filesystem::path FindFont(std::string name);
	std::vector<std::pair<std::string, Font>> AvailableFonts;
	std::unordered_map<std::string, ElementFont> LoadedFonts;
private:
	void UpdateFontsPath(std::filesystem::path p);
	std::vector<ElementFont> FontsToLoad;
	bool fonts_need_loaded;
};