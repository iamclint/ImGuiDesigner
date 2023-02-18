#include "Workspace.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
void WorkSpace::OnUIRender() {
	ImGui::Begin("WorkSpace");
	
	if (elements_buffer.size() > 0)
	{
		for (auto& element : elements_buffer)
		{
			elements.push_back(element);
		}
		elements_buffer.clear();
	}
	
	for (auto& element : elements)
	{
		element->Render();
	}
	
	//delete from elements if delete_me is true
	for (auto it = elements.begin(); it != elements.end();)
	{
		if ((*it)->v_parent)
		{
			(*it)->v_parent->children.push_back(*it);
			it = elements.erase(it);
		}
		else if ((*it)->delete_me)
		{
			delete (*it);
			it = elements.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	ImGui::End();
}