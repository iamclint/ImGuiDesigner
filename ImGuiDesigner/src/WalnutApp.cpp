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
	std::vector<WorkSpace*> delete_workspace;
	std::vector<ImGuiElement> undo_vector;
	FontManager* font_manager;
	std::filesystem::path startup_path;
}

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();
	}
};


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
	igd::app->SetMenubarCallback([]()
	{			
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				igd::notifications->OpenFile([](std::string file) {
					igd::active_workspace->Open(file);
					});
			}
			if (ImGui::MenuItem("Save"))
			{
				igd::notifications->SaveFile([](std::string file) {
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