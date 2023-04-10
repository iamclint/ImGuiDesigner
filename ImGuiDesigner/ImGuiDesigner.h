#pragma once
#include "Workspace.h"
#include "Properties.h"
#include "ToolBar.h"
#include "Dialogs.h"
#include "FontManager.h"
#include "Settings.h"
#include <filesystem>
#include "Walnut/Image.h"
namespace igd
{
	template <typename t> void VecMove(std::vector<t>& v, size_t oldIndex, size_t newIndex)
	{
		if (v.size() > 0 )
		{
			if (oldIndex > newIndex)
				std::rotate(v.rend() - oldIndex - 1, v.rend() - oldIndex, v.rend() - newIndex);
			else
			{
				if (newIndex >= v.size())
					std::rotate(v.begin() + oldIndex, v.begin() + oldIndex, v.begin() + newIndex);
				else
					std::rotate(v.begin() + oldIndex, v.begin() + oldIndex + 1, v.begin() + newIndex + 1);
			}
		}
	}

	void UnPressKey(ImGuiKey key);
	void push_designer_theme();
	void pop_designer_theme();
	extern Dialogs* dialogs;
	extern WorkSpace* active_workspace;
	extern std::vector<WorkSpace*> workspaces;
	extern Properties* properties;
	extern Walnut::Application* app;
	extern bool add_workspace;
	extern std::vector<WorkSpace*> delete_workspace;
	extern FontManager* font_manager;
	extern std::filesystem::path startup_path;
	extern std::string open_file;
	extern ImFont* designer_font;
	extern std::unordered_map<const char*, ImFont*>* designer_fonts;
	extern Settings* settings;
	void onUpdate();
}