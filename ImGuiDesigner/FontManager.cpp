#include "FontManager.h"
#include <Windows.h>
#include <filesystem>
#include "ImGuiDesigner.h"
#include <iostream>
#include <fstream>
#include <bit>
#include <cstdint>

struct cmap_subtable_header {
	uint16_t platform_id;
	uint16_t encoding_id;
	uint32_t offset;
	cmap_subtable_header() : platform_id(0), encoding_id(0), offset(0) {}
	cmap_subtable_header(std::ifstream& file)
	{
		encoding_id = 0;
		offset = 0;
		platform_id = 0;
		file.read(reinterpret_cast<char*>(&platform_id), sizeof(platform_id));
		file.read(reinterpret_cast<char*>(&encoding_id), sizeof(encoding_id));
		file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
		encoding_id = std::byteswap(encoding_id);
		offset = std::byteswap(offset);
		platform_id = std::byteswap(platform_id);
	}
};
struct ttf_header
{
	uint32_t sfntVersion;
	uint16_t numTables;
	uint16_t searchRange;
	uint16_t entrySelector;
	uint16_t rangeShift;
	ttf_header() : sfntVersion(0), numTables(0), searchRange(0), entrySelector(0), rangeShift(0) {}
	ttf_header(std::ifstream& file)
	{
		sfntVersion = 0;
		numTables = 0;
		searchRange = 0;
		entrySelector = 0;
		rangeShift = 0;
		file.read(reinterpret_cast<char*>(&sfntVersion), sizeof(sfntVersion));
		file.read(reinterpret_cast<char*>(&numTables), sizeof(numTables));
		file.read(reinterpret_cast<char*>(&searchRange), sizeof(searchRange));
		file.read(reinterpret_cast<char*>(&entrySelector), sizeof(entrySelector));
		file.read(reinterpret_cast<char*>(&rangeShift), sizeof(rangeShift));
		sfntVersion = std::byteswap(sfntVersion);
		numTables = std::byteswap(numTables);
		searchRange = std::byteswap(searchRange);
		entrySelector = std::byteswap(entrySelector);
		rangeShift = std::byteswap(rangeShift);
	}
};
struct ttf_table
{
	char tag[5];
	uint32_t checkSum;
	uint32_t offset;
	uint32_t length;
	ttf_table(std::ifstream& file)
	{
		tag[0] = 0;
		tag[1] = 0;
		tag[2] = 0;
		tag[3] = 0;
		tag[4] = 0;
		checkSum = 0;
		offset = 0;
		length = 0;
		file.read(tag, 4);
		file.read(reinterpret_cast<char*>(&checkSum), sizeof(checkSum));
		file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
		file.read(reinterpret_cast<char*>(&length), sizeof(length));
		checkSum = std::byteswap(checkSum);
		offset = std::byteswap(offset);
		length = std::byteswap(length);
	}
	ttf_table() : checkSum(0), offset(0), length(0)
	{
		tag[0] = 0;
		tag[1] = 0;
		tag[2] = 0;
		tag[3] = 0;
		tag[4] = 0;
	}
};
//function to check if ttf contains an unicode index map
bool Font::hasUnicodeEncoding(std::filesystem::path filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Error opening file " << filePath << std::endl;
		return false;
	}
	ttf_header header(file);
	for (uint32_t i = 0; i < header.numTables; i++) {

		ttf_table tbl(file);
		if (std::string(tbl.tag) == "cmap") 
		{
			file.seekg(tbl.offset+2, std::ios::beg);
			uint16_t subTables=0;
			file.read(reinterpret_cast<char*>(&subTables), sizeof(subTables));
			subTables = std::byteswap(subTables);
			for (uint32_t x = 0; x < subTables; x++) {
				cmap_subtable_header header(file);
				if (header.platform_id == 3 && header.encoding_id == 1)
				{
					file.close();
					return true;
				}
			}
			file.close();
			return false;
		}
	}
	file.close();
	return false;
}

void FontManager::UpdateFontsPath(std::filesystem::path in_path)
{
	for (auto& p : std::filesystem::directory_iterator(in_path))
	{
		if (p.path().extension() == ".ttf")
		{
			if (AvailableFonts.find(p.path().filename().stem().string()) == AvailableFonts.end())
			{
				AvailableFonts[p.path().filename().stem().string()] = Font(p.path());
			}
		}
	}
}
void FontManager::UpdateFonts()
{
	//create directory if doesn't exist
	std::filesystem::path fonts_dir = igd::startup_path.string() + "/fonts";
	if (!std::filesystem::exists(fonts_dir))
		std::filesystem::create_directory(fonts_dir);
	UpdateFontsPath(igd::font_manager->GetWindowsFontsDirectory());
	UpdateFontsPath(fonts_dir);
}

std::filesystem::path FontManager::GetWindowsFontsDirectory()
{
	//get windows font directory
	char windir[MAX_PATH];
	//GetWindowsDirectoryA return validation
	if (GetWindowsDirectoryA(windir, MAX_PATH))
	{
		std::filesystem::path path = windir;
		path /= "Fonts";
		return path;
	}
	else
	{
		std::cout << "Failed to get windows directory" << std::endl;
		return "";
	}
	//GetWindowsDirectoryA(windir, MAX_PATH);
	//std::string fontdir = windir;
	//fontdir += "\\Fonts\\";
	return "";
}

std::filesystem::path FontManager::FindFont(std::string name)
{
	std::filesystem::path fontdir = GetWindowsFontsDirectory();
	for (auto& p : std::filesystem::directory_iterator(fontdir))
	{
		if (p.path().filename().string() == name)
			return p.path();
	}
	fontdir = "fonts";
	for (auto& p : std::filesystem::directory_iterator(fontdir))
	{
		if (p.path().filename().string() == name)
			return p.path();
	}
	return "";
}

ElementFont* FontManager::GetFont(std::string name, int size)
{
	return &this->LoadedFonts[name + ":" + std::to_string(size)];
}
void FontManager::LoadFont(std::filesystem::path path, int size, ImGuiElement* element)
{
	this->FontsToLoad.push_back({ path, size, element });
	this->fonts_need_loaded = true;
}

void FontManager::OnUpdate(float ssa)
{
	if (this->fonts_need_loaded)
	{
		for (auto& f : this->FontsToLoad)
		{
			std::cout << "Loading font: " << f.name << " " << f.path << std::endl;
			if (!this->LoadedFonts[f.map_name].font)
				LoadedFonts[f.map_name].font = ImGui::GetIO().Fonts->AddFontFromFileTTF(f.path.string().c_str(), (float)f.size);

			if (f.element)
			{
				f.element->v_font.name = f.name;
				f.element->v_font.path = f.path;
				f.element->v_font.size = f.size;
				f.element->v_font.font = LoadedFonts[f.map_name].font;
				f.element->PushUndo();
			}
			this->fonts_need_loaded = false;
			ImGui::GetIO().Fonts->Build();
		}
		this->FontsToLoad.clear();
		igd::app->UpdateFonts();
	}
}
void FontManager::OnUIRender()
{
	
}