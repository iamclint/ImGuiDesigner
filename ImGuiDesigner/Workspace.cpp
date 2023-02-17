#include "Workspace.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
void WorkSpace::OnUIRender() {
	ImGui::Begin("WorkSpace");
	for (auto& element : elements)
	{
		element->Render();
	}
	ImGui::End();
}