#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "..\ToolBar.h"
#include "..\Properties.h"
#include "..\Workspace.h"

namespace igd
{ 
	WorkSpace* work;
	Properties* properties;
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


Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "ImGui Designer";

	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<Properties> properties = std::make_shared<Properties>();
	std::shared_ptr<WorkSpace> work = std::make_shared<WorkSpace>();
	igd::work = work.get();
	igd::properties = properties.get();
	app->PushLayer<ToolBar>();
	app->PushLayer(work);
	app->PushLayer(properties);
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}