#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "..\ToolBar.h"
#include "..\Properties.h"
#include "..\Workspace.h"
#include "..\ImGuiDesigner.h"

#include <iostream>

namespace igd
{ 
	WorkSpace* active_workspace;
	std::vector<WorkSpace*> workspaces;
	Properties* properties;
	Notifications* notifications;
	Walnut::Application* app;
	bool add_workspace = false;
	std::string open_file="";
	std::vector<WorkSpace*> delete_workspace;
	std::vector<ImGuiElement> undo_vector;
	FontManager* font_manager;
	std::filesystem::path startup_path;
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
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.281853, 0.281853, 0.281853, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.316602, 0.316602, 0.316602, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4, 4));
	}
	void pop_designer_theme()
	{
		ImGui::PopStyleVar(4);
		ImGui::PopStyleColor(10);
	}
}

void update_layer_stack()
{
	if (igd::delete_workspace.size()>0)
	{
		igd::active_workspace->active_element = nullptr;
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
	igd::app = new Walnut::Application(spec);
	std::shared_ptr<Properties> properties = std::make_shared<Properties>();
	std::shared_ptr<WorkSpace> work = std::make_shared<WorkSpace>();
	std::shared_ptr<Notifications> notifications = std::make_shared<Notifications>();
	std::shared_ptr<FontManager> font_manager = std::make_shared<FontManager>();
	igd::active_workspace = work.get();
	igd::workspaces.push_back(igd::active_workspace);
	igd::properties = properties.get();
	igd::notifications = notifications.get();
	igd::font_manager = font_manager.get();
	igd::app->PushLayer<ToolBar>();
	igd::app->PushLayer(work);
	igd::app->PushLayer(properties);
	igd::app->PushLayer(notifications);
	igd::app->PushLayer(font_manager);
	igd::app->SetUpdateLayerStackCallback(update_layer_stack);
	igd::font_manager->UpdateFonts();
	igd::app->SetMenubarCallback([]()
	{			
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				igd::notifications->OpenFile([](std::string file) {
					igd::open_file = file;
					igd::add_workspace = true;
				});
			}
			if (ImGui::MenuItem("Save"))
			{
				igd::notifications->SaveFile([](std::string file) {
					//check if file exists
					if (std::filesystem::exists(file))
					igd::notifications->Confirmation("Overwrite File", "Are you sure you wish to overwrite\n" + file, "", [file](bool result) {
						if (result)
							igd::active_workspace->Save(file);
					});
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
	});
	return igd::app;
}