#include "..\includes\Application.h"
#include "..\includes\EntryPoint.h"
#include "..\ToolBar.h"
#include "..\Properties.h"
#include "..\Workspace.h"
#include "..\ImGuiDesigner.h"
#include "GLFW/glfw3.h"
#include "..\textures\icon.embed"
#include "..\textures\logo.embed"
#include "..\includes\stb_image.h"
#include "..\fonts\pt.embed"
#include <iostream>

namespace igd
{ 
	WorkSpace* active_workspace;
	std::vector<WorkSpace*> workspaces;
	Properties* properties;
	Dialogs* dialogs;
	Settings* settings=new Settings();
	Walnut::Application* app;
	bool add_workspace = false;
	std::string open_file="";
	std::vector<WorkSpace*> delete_workspace;
	std::vector<ImGuiElement> undo_vector;
	FontManager* font_manager;
	std::filesystem::path startup_path;
	ImFont* designer_font;
	Textures textures;
	std::unordered_map<const char*, ImFont*>* designer_fonts;

	//get 2d distance between 2 vec2
	float GetDistance(ImVec2 a, ImVec2 b)
	{
		return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
	}

	ImGuiElement* GetNearestElement(ImRect& rect, ImGuiElement* element)
	{
		float max_dist = FLT_MAX;
		ImGuiElement* nearest = nullptr;
		ImGuiElement* parent = element->v_parent;
		if (!parent)
			parent = igd::active_workspace->basic_workspace_element;

		for (auto& e : parent->children)
		{
			if (e == element || e == parent || e->delete_me || e->v_is_dragging)
				continue;

			float current_dist = GetDistance(rect.Min+(rect.GetSize()/2), e->GetPos() + ImVec2(e->GetSize() / 2));
			if (current_dist < max_dist)
			{
				nearest = e;
				max_dist = current_dist;
			}
		}
		return nearest;
	}

	RectSide getNearestSide(const ImRect& rect1, const ImRect& rect2, float threshold)
	{
		float leftDist = fabsf(rect1.Min.x - rect2.Max.x);
		float rightDist = fabsf(rect1.Max.x - rect2.Min.x);
		float topDist = fabsf(rect1.Min.y - rect2.Max.y);
		float bottomDist = fabsf(rect1.Max.y - rect2.Min.y);

		if (topDist <= bottomDist && topDist <= threshold && rect1.Min.y >= rect2.Max.y)
		{
			return RectSide::Top;
		}
		else if (bottomDist <= topDist && bottomDist <= threshold && rect1.Max.y <= rect2.Min.y)
		{
			return RectSide::Bottom;
		}
		else if (leftDist <= rightDist && leftDist <= threshold)
		{
			return RectSide::Left;
		}
		else if (rightDist <= leftDist && rightDist <= threshold)
		{
			return RectSide::Right;
		}
		else
		{
			return RectSide::Bottom;
		}
	}


	ImGuiElement* GetNearestElement(ImGuiElement* element, bool use_cursor_pos)
	{
		float current_nearest_dist = FLT_MAX;
		ImGuiElement* nearest=nullptr;
		ImGuiElement* parent = element->v_parent;
		if (!parent)
			parent = igd::active_workspace->basic_workspace_element;
		ImRect r = element->GetItemRect();
		if (use_cursor_pos)
		{
			ImVec2 mp = ImGui::GetMousePos();
			r = { mp, mp + ImVec2(10,10) };
		}
		for (auto& e : parent->children)
		{
			if (e == element || e == parent || e->delete_me || e->v_is_dragging)
				continue;
			//float current_dist = std::min(std::min(std::abs(e->GetItemRect().Min.x - element->GetItemRect().Min.x), std::abs(e->GetItemRect().Max.x - element->GetItemRect().Max.x)),
		//		std::min(std::abs(e->GetItemRect().Min.y - element->GetItemRect().Min.y), std::abs(e->GetItemRect().Max.y - element->GetItemRect().Max.y)));


			//float distanceLeft = abs(r.Min.x - e->GetItemRect().Max.x);
			//float distanceRight = abs(r.Max.x - e->GetItemRect().Min.x);
			//float distanceTop = abs(r.Min.y - e->GetItemRect().Max.y);
			//float distanceBottom = abs(r.Max.y - e->GetItemRect().Min.y);

			//calc 2d distance from every corner
			float distanceLeft = sqrt(pow(e->GetItemRect().Min.x - r.Min.x, 2) + pow(e->GetItemRect().Min.y - r.Min.y, 2));
			float distanceRight = sqrt(pow(e->GetItemRect().Max.x - r.Max.x, 2) + pow(e->GetItemRect().Max.y - r.Max.y, 2));
			float distanceTop = sqrt(pow(e->GetItemRect().Min.x - r.Min.x, 2) + pow(e->GetItemRect().Max.y - r.Max.y, 2));
			float distanceBottom = sqrt(pow(e->GetItemRect().Max.x - r.Max.x, 2) + pow(e->GetItemRect().Min.y - r.Min.y, 2));
			

			// Check if any of the distances are smaller than the current closest distance
			if (distanceLeft < current_nearest_dist && distanceLeft) {
				nearest = e;
				current_nearest_dist = distanceLeft;
			}
			if (distanceRight < current_nearest_dist && distanceRight) {
				nearest = e;
				current_nearest_dist = distanceRight;
			}
			if (distanceTop < current_nearest_dist && distanceTop) {
				nearest = e;
				current_nearest_dist = distanceTop;
			}
			if (distanceBottom < current_nearest_dist && distanceBottom) {
				nearest = e;
				current_nearest_dist = distanceBottom;
			}

			//float current_dist = std::abs(e->GetItemRect().Min.x - element->GetItemRect().Min.x) + std::abs(e->GetItemRect().Min.y - element->GetItemRect().Min.y)
			//	+ std::abs(e->GetItemRect().Max.x - element->GetItemRect().Max.x) + std::abs(e->GetItemRect().Max.y - element->GetItemRect().Max.y);
			////current_dist =  GetDistance(element->GetPos() + ImVec2(element->GetRawSize() / 2), e->GetPos() + ImVec2(e->GetRawSize()/2));

			//if (current_dist < current_nearest_dist && current_dist>0)
			//{
			//	nearest = e;
			//	current_nearest_dist = current_dist;
			//}
		}
		return nearest;
	}

	bool doRectanglesIntersect(const ImRect& rect1, const ImRect& rect2)
	{
		// Ensure that the coordinates are in the correct order
		float left1 = std::min(rect1.Min.x, rect1.Max.x);
		float top1 = std::min(rect1.Min.y, rect1.Max.y);
		float right1 = std::max(rect1.Min.x, rect1.Max.x);
		float bottom1 = std::max(rect1.Min.y, rect1.Max.y);

		float left2 = std::min(rect2.Min.x, rect2.Max.x);
		float top2 = std::min(rect2.Min.y, rect2.Max.y);
		float right2 = std::max(rect2.Min.x, rect2.Max.x);
		float bottom2 = std::max(rect2.Min.y, rect2.Max.y);

		// Check if the rectangles intersect
		return left1 <= right2 && right1 >= left2 && top1 <= bottom2 && bottom1 >= top2;
	}

	std::string WordWrap(std::string& input, int char_limit)
	{
		int len = input.length();
		if (len <= char_limit)
			return input;

		std::stringstream output;
		int i = 0;
		while (i < len)
		{
			if (i + char_limit < len)
			{
				//find the next space in case the line break is in the middle of a word
				int next_space = input.substr(i + char_limit).find(' ');
				if (next_space != std::string::npos)
				{
					output << input.substr(i, char_limit + next_space) << std::endl;
					i += next_space + 1;
				}
				else
					output << input.substr(i, char_limit) << std::endl;
			}
			i += char_limit;
			if (i >= len)
				output << input.substr(i - char_limit);
		}
		return output.str();
	}


	// ImageButton() is flawed as 'id' is always derived from 'texture_id' (see #2464 #1390)
// We provide this internal helper to write your own variant while we figure out how to redesign the public ImageButton() API.
	bool ImageButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& full_rect_size, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGui::KeepAliveID(id);

		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + full_rect_size);
		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(full_rect_size.x, full_rect_size.y), 0.0f, g.Style.FrameRounding));
		if (bg_col.w > 0.0f)
			window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_col));
		window->DrawList->AddImage(texture_id, bb.Min + ((full_rect_size - size) / 2), bb.Max - ((full_rect_size - size) / 2), uv0, uv1, GetColorU32(tint_col));

		return pressed;
	}
	bool ImageButtonTextEx(ImGuiID id, ImTextureID texture_id, const ImVec2& size, std::string& text, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& full_rect_size, const ImVec4& bg_col, const ImVec4& tint_col)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;
		ImGui::KeepAliveID(id);

		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + full_rect_size);
		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held);

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(full_rect_size.x, full_rect_size.y), 0.0f, g.Style.FrameRounding));
		if (bg_col.w > 0.0f)
			window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_col));

		
		ImRect ImageRect = { bb.Min + ImVec2(10, ((full_rect_size.y - size.y) / 2)), bb.Min + ImVec2(size.x + 10, (((full_rect_size.y - size.y) / 2) + size.y)) };
		window->DrawList->AddImage(texture_id, ImageRect.Min, ImageRect.Max, uv0, uv1, GetColorU32(tint_col));
		ImVec2 TextPos = ImVec2(ImageRect.Max.x+10, bb.Min.y+(((bb.Max.y-bb.Min.y)- ImGui::CalcTextSize(text.c_str()).y)/2));// ((ImageRect.Max.y - ImageRect.Min.y) - ImGui::CalcTextSize(text.c_str()).y) / 2);
		window->DrawList->AddText(TextPos, GetColorU32(ImGuiCol_Text),text.c_str());

		return pressed;
	}
	bool ImageButtonText(ImTextureID user_texture_id, const ImVec2& size, std::string text, ImVec2 full_size, ImColor bg_color)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		text = WordWrap(text, 10);
		ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
		if (full_size.y < text_size.y)
			full_size.y = text_size.y;


		// Default to using texture ID as ID. User can still push string/integer prefixes.
		ImGui::PushID((void*)(intptr_t)user_texture_id);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();
		const ImVec2& uv0 = ImVec2(0, 0);
		const ImVec2& uv1 = ImVec2(1, 1);
		int frame_padding = -1;
		const ImVec4& tint_col = ImVec4(1, 1, 1, 1);
		const ImVec2 padding = full_size;
		return ImageButtonTextEx(id, user_texture_id, size, text, uv0, uv1, full_size, bg_color, tint_col);
	}
	bool ImageButton(ImTextureID user_texture_id, const ImVec2& size, ImVec2 full_size, ImColor bg_color)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		// Default to using texture ID as ID. User can still push string/integer prefixes.
		ImGui::PushID((void*)(intptr_t)user_texture_id);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();
		const ImVec2& uv0 = ImVec2(0, 0);
		const ImVec2& uv1 = ImVec2(1, 1);
		int frame_padding = -1;
		const ImVec4& tint_col = ImVec4(1, 1, 1, 1);
		const ImVec2 padding = full_size;

		return ImageButtonEx(id, user_texture_id, size, uv0, uv1, full_size, bg_color, tint_col);
	}



	void onUpdate()
	{
		//if (!designer_font)
		//{
		//	designer_font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)embedded::arial, 17, 17);
		//	igd::app->UpdateFonts();
		//}
	}
	
	void UnPressKey(ImGuiKey key)
	{
		ImGuiKeyData* key_data = ImGui::GetKeyData(key);
		key_data->DownDuration = -1.f;
		key_data->Down = false;
		ImGui::GetIO().KeysDown[key] = false;
	}
	void push_designer_theme()
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.192782, 0.204633, 0.202574, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.281853, 0.281853, 0.281853, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.316602, 0.316602, 0.316602, 1));
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.199288, 0.199288, 0.199288, 1));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.199288, 0.199288, 0.199288, 1));
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.199288, 0.199288, 0.199288, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.316602, 0.316602, 0.316602, 1));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(31, 31, 31, 255).Value);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.281853, 0.281853, 0.281853, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.316602, 0.316602, 0.316602, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4, 4));
		//if (designer_font)
		//	ImGui::PushFont(designer_font);
		
	//	ImGui::PushFont();
	}
	void pop_designer_theme()
	{
		ImGui::PopStyleVar(4);
		ImGui::PopStyleColor(11);
	/*	if (designer_font)
			ImGui::PopFont();*/
	}
}

void update_layer_stack()
{
	if (igd::delete_workspace.size()>0)
	{

		igd::active_workspace->selected_elements.clear();
		for (auto& w : igd::delete_workspace)
		{
			std::cout << "Deleting workspace" << std::endl;
			if (w == igd::active_workspace)
			{
				if (igd::workspaces.size() > 1)
					igd::active_workspace = igd::workspaces[1];
				else
					igd::active_workspace = nullptr;
			}
			igd::workspaces.erase(std::remove(igd::workspaces.begin(), igd::workspaces.end(), w), igd::workspaces.end());
			igd::app->PopLayer(igd::app->GetLayerByPtr(w));
		}
		igd::delete_workspace.clear();
	}
	
	if (igd::add_workspace)
	{
		std::cout << "Adding workspace" << std::endl;
		igd::add_workspace = false;
		std::shared_ptr<WorkSpace> work = std::make_shared<WorkSpace>();
		igd::active_workspace = work.get();
		igd::workspaces.push_back(igd::active_workspace);
		igd::app->PushLayer(work);

		if (igd::open_file!="")
			igd::active_workspace->Open(igd::open_file);
		igd::open_file = "";
	}
}


void create_directories()
{
	//get startup path
	igd::startup_path = std::filesystem::current_path();
	
	std::filesystem::path path = std::filesystem::current_path();
	path.append("textures");
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}
	path = std::filesystem::current_path();
	path.append("fonts");
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}
	path = std::filesystem::current_path();
	path.append("widgets");
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}
	path = std::filesystem::current_path();
	path.append("projects");
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}
	path = std::filesystem::current_path();
	path.append("palettes");
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}
}

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	create_directories();
	Walnut::ApplicationSpecification spec;
	spec.Name = "ImGui Designer";
	spec.Width = 1600;
	spec.Height = 800;
	spec.Font = (void*)embedded::pt;
	spec.FontDataSize = sizeof(embedded::pt);
	spec.FontSize = 15;
	igd::app = new Walnut::Application(spec);
	GLFWimage Images[1];
	Images[0].pixels = stbi_load_from_memory(embedded::icon, sizeof(embedded::icon), &Images[0].width, &Images[0].height, 0, 4);
	glfwSetWindowIcon(igd::app->GetWindowHandle(), 1, Images);
	stbi_image_free(Images[0].pixels);
	igd::textures.init();
	std::shared_ptr<Properties> properties = std::make_shared<Properties>();
	std::shared_ptr<WorkSpace> work = std::make_shared<WorkSpace>();
	std::shared_ptr<Dialogs> dialogs = std::make_shared<Dialogs>();
	std::shared_ptr<FontManager> font_manager = std::make_shared<FontManager>();
	font_manager->LoadFont(spec.Font, spec.FontDataSize, "designer", 12, nullptr);
	font_manager->LoadFont(spec.Font, spec.FontDataSize, "designer", 14, nullptr);
	font_manager->LoadFont(spec.Font, spec.FontDataSize, "designer", 20, nullptr);
	font_manager->LoadFont(spec.Font, spec.FontDataSize, "designer", 24, nullptr);
	font_manager->LoadFont(spec.Font, spec.FontDataSize, "designer", 28, nullptr);
	font_manager->LoadFont(spec.Font, spec.FontDataSize, "designer", 36, nullptr);
	igd::active_workspace = work.get();
	igd::workspaces.push_back(igd::active_workspace);
	igd::properties = properties.get();
	igd::dialogs = dialogs.get();
	igd::font_manager = font_manager.get();
	igd::settings->load();

	igd::app->PushLayer<ToolBar>();
	igd::app->PushLayer(work);
	igd::app->PushLayer(properties);
	igd::app->PushLayer(dialogs);
	igd::app->PushLayer(font_manager);
	igd::app->SetUpdateLayerStackCallback(update_layer_stack);
	igd::font_manager->UpdateFonts();
	igd::app->SetMenubarCallback([]()
	{			
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				igd::dialogs->OpenFile([](std::string file) {
					igd::open_file = file;
					igd::add_workspace = true;
				});
			}
			if (ImGui::MenuItem("Save"))
			{
				igd::dialogs->SaveFile([](std::string file) {
					if (std::filesystem::exists(file))
					{
						igd::dialogs->Confirmation("Overwrite File", "Are you sure you wish to overwrite\n" + file, "", [file](bool result) {
							if (result)
								igd::active_workspace->Save(file);
							});
					}
					else
						igd::active_workspace->Save(file);
				});
			}
			if (ImGui::MenuItem("Exit"))
			{
				igd::app->Close();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Settings"))
			{
				igd::dialogs->ShowSettings();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				igd::dialogs->ShowSettings();
			}
			ImGui::EndMenu();
		}
	});

	return igd::app;
}