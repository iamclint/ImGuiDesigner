#pragma once
#include "Workspace.h"
#include "Properties.h"
#include "ToolBar.h"
#include "Dialogs.h"
#include "FontManager.h"
#include "Settings.h"
#include <filesystem>
#include "Walnut/Image.h"
#include "Textures.h"
#include "igd_imgui_widgets.h"
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
	std::string WordWrap(std::string& text, int max_chars);
	void UnPressKey(ImGuiKey key);
	void push_designer_theme();
	void pop_designer_theme();

	bool ImageButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& full_rect_size, const ImVec4& bg_col, const ImVec4& tint_col);
	bool ImageButtonTextEx(ImGuiID id, ImTextureID texture_id, const ImVec2& size, std::string& text, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& full_rect_size, const ImVec4& bg_col, const ImVec4& tint_col);
	bool ImageButton(ImTextureID user_texture_id, const ImVec2& size, ImVec2 full_size = { 60, 60 }, ImColor bg_color={0,0,0,0});
	bool ImageButtonText(ImTextureID user_texture_id, const ImVec2& size, std::string text, ImVec2 full_size = { 60, 60 }, ImColor bg_color = { 0,0,0,0 });
	bool doRectanglesIntersect(const ImRect& rect1, const ImRect& rect2);
	ImGuiElement* GetNearestElement(ImGuiElement* element, bool use_custom_rect = false, ImRect custom_rect={ImVec2(0,0),ImVec2(0,0)});
	float GetDistance(ImVec2 a, ImVec2 b);
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
	extern Textures textures;
	void onUpdate();
}