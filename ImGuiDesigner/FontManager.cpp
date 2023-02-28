#include "FontManager.h"
#include <Windows.h>
#include <filesystem>
#include "ImGuiDesigner.h"
#include <iostream>
std::filesystem::path FontManager::GetWindowsFontsDirectory()
{
	//get windows font directory
	char windir[MAX_PATH];
	GetWindowsDirectoryA(windir, MAX_PATH);
	std::string fontdir = windir;
	fontdir += "\\Fonts\\";
	return fontdir;
}

std::filesystem::path FontManager::FindFont(std::string name)
{
	std::filesystem::path fontdir = GetWindowsFontsDirectory();
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
			if (!this->LoadedFonts[f.name].font)
				LoadedFonts[f.name].font = ImGui::GetIO().Fonts->AddFontFromFileTTF(f.path.string().c_str(), f.size);

			if (f.element)
			{
				f.element->v_font.name = f.name;
				f.element->v_font.path = f.path;
				f.element->v_font.size = f.size;
				f.element->v_font.font = LoadedFonts[f.name].font;
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