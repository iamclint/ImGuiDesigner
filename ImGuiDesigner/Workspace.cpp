#include "Workspace.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
void WorkSpace::OnUIRender() {
	ImGui::Begin("WorkSpace");
	ImGui::Text("Start work space");
	ImGui::Button("Button##toolbar_input_button");
	ImGui::End();
}