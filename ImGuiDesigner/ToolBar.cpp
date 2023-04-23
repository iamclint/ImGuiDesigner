#include "ToolBar.h"
#include "Walnut/Image.h"
#include "imgui_internal.h"
#include "Workspace.h"
#include "Properties.h"
#include "ImGuiDesigner.h"
#include "../json/single_include/nlohmann/json.hpp"
#include <fstream>
#include "igd_elements.h"



ToolBar::ToolBar()
{

}

template<typename T>
bool ToolBar::Tool(std::string name, ImVec2 size, bool handle_click, std::string override_tooltip)
{
	T ref_element;
	ImGuiElement* ref = (ImGuiElement*)&ref_element;
	ImGuiElement* active = igd::active_workspace->GetSingleSelection();
	ImGuiContext& g = *GImGui;
	bool clicked = false;
	if (ref->v_parent_required_id && !active)
		return false;
	else if (ref->v_parent_required_id && !(ref->v_parent_required_id & active->v_type_id))
		return false;
	if (!active || !active->v_element_filter || (active->v_element_filter && (active->v_element_filter & ref->v_type_id)))
	{
		ImGui::TableNextColumn();
		if (ref->v_icon && ref->v_icon->GetDescriptorSet())
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor(65, 67, 74, 65).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(65, 67, 74, 255).Value);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(65, 67, 74, 255).Value);
			ImGui::PushID(name.c_str());
			if (igd::ImageButton(ref->v_icon->GetDescriptorSet(), ref->v_icon->GetSize() / 2))
				clicked = true;
			ImGui::PopID();
//			ImGui::Image(ref->v_icon->GetDescriptorSet(), ref->v_icon->GetSize()/2);

			ImGui::PopStyleColor(3);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				ImGui::BeginTooltip();
				if (override_tooltip!="")
					ImGui::Text(override_tooltip.c_str());
				else
					ImGui::Text(ref->v_tooltip.c_str());
				ImGui::EndTooltip();
			}
			
		}
		else
			clicked = ImGui::Button(name.c_str(), size);
		if (clicked && handle_click)
		{
			if (ref->v_type_id == (int)element_type::texture)
			{
				igd::dialogs->OpenFile([ref](std::string file) {
					igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::Texture(file)), false, ref->v_auto_select);
					}, "PNG\0*.png\0JPG\0*.jpg\0All Files(*.*)\0*.*\0\0");
			}
			else
			{
				igd::active_workspace->AddNewElement((ImGuiElement*)(new T()), false, ref->v_auto_select);
			}
			return true;
		}
	}
	return clicked;
}
void ToolBar::RenderElements()
{

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImVec2 size = { 140, 30 };
	ImGui::BeginTable("##toolbar", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedSame);
	
	this->Tool<igd::ChildWindow>("Child Window", size);
	this->Tool<igd::Button>("Button", size);
	

	if (this->Tool<igd::InputText>("Input Text", size, false, "Inputs"))
	{
		ImGui::OpenPopupEx(window->GetID("InputGroup"));
	}

	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImColor(32, 32, 34, 85).Value);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if (ImGui::BeginPopupEx(window->GetID("InputGroup"), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
	{
		//ImGui::Image(igd::textures.images["input"]->GetDescriptorSet(), igd::textures.images["input"]->GetSize()/2);
		if (ImGui::Selectable("Input Text"))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::InputText()), false, true);
		if (ImGui::Selectable("Input Int"))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::InputInt()), false, true);
		if (ImGui::Selectable("Input Float"))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::InputFloat()), false, true);
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	if (this->Tool<igd::SliderInt>("Slider Int", size, false, "Sliders"))
	{
		ImGui::OpenPopupEx(window->GetID("SliderGroup"));
	}

	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImColor(32, 32, 34, 85).Value);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if (ImGui::BeginPopupEx(window->GetID("SliderGroup"), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
	{
		//ImGui::Image(igd::textures.images["input"]->GetDescriptorSet(), igd::textures.images["input"]->GetSize()/2);
		if (ImGui::Selectable("Slider Int"))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::SliderInt()), false, true);
		if (ImGui::Selectable("Slider Float"))
			igd::active_workspace->AddNewElement((ImGuiElement*)(new igd::SliderFloat()), false, true);
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	this->Tool<igd::CheckBox>("Checkbox", size);
	this->Tool<igd::Text>("Text", size);
	this->Tool<igd::Separator>("Separator", size);
	this->Tool<igd::Combo>("Combo", size);
	this->Tool<igd::Selectable>("Selectable", size);
	this->Tool<igd::Table>("Table", size);
	this->Tool<igd::TabBar>("TabBar", size);
	this->Tool<igd::TabItem>("TabItem", size);
	this->Tool<igd::Texture>("Texture", size);
	ImGui::EndTable();
	//ImGui::Text("path: %s", std::filesystem::current_path().string().c_str());
}

void ToolBar::UpdateWidgets()
{
	for (auto& p : std::filesystem::directory_iterator(igd::startup_path.string() + "/widgets"))
	{
		if (p.path().extension() == ".wgd")
		{
			//if path doesn't exist in widgets map, load it
			if (widgets.find(p.path()) == widgets.end())
			{
				std::ifstream i(p.path().string());
				try
				{
					nlohmann::json j;
					i >> j;
					if (!j["obj"].contains("desc"))
						j["obj"]["desc"] = "";

					if (j["obj"].contains("icon_name"))
						widgets[p.path()] = widget(p.path(), j["obj"]["name"], j["obj"]["desc"], nullptr, j["obj"]["icon_name"]);
					else
						widgets[p.path()] = widget(p.path(), j["obj"]["name"], j["obj"]["desc"], nullptr, "");
				}
				catch (nlohmann::json::exception& ex)
				{
					//igd::dialogs->GenericNotification("Json Error", std::string(ex.what()) + "\n" + p.path().string(), "", "Ok", []() {});
					std::cerr << "parse error at byte " << ex.what() << std::endl;
				}
				catch (nlohmann::json::parse_error& ex)
				{
					//igd::dialogs->GenericNotification("Json Error", std::string(ex.what()) + "\n" + p.path().string(), "", "Ok", []() {});
					std::cerr << "parse error at byte " << ex.byte << std::endl << ex.what() << std::endl;
				}
				catch (nlohmann::json::type_error& ex)
				{
	
					igd::dialogs->GenericNotification("Json Error", std::string(ex.what()) + "\n" + p.path().string(), "", "Ok", []() {});
				}
			}
		}
	}
}


void ToolBar::RenderCustomWidgets()
{
	UpdateWidgets();
	ImGuiContext& g = *GImGui;
	//iterate all files in widgets folder
	ImGui::PushStyleColor(ImGuiCol_Button, ImColor(65, 67, 74, 65).Value);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(65, 67, 74, 255).Value);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(65, 67, 74, 255).Value);
	//ImGui::BeginTable("##toolbar", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit);
	for (int i = 0;auto& [path, w] : widgets)
	{

		std::string name= w.file.filename().stem().string();
		if (w.name != "")
			name = w.name + "##" + w.name + std::to_string(i);
		//ImGui::TableNextColumn();
		//check if exists in map
		bool clicked = false;
		if (w.icon_name != "" && igd::textures.images.find(w.icon_name) != igd::textures.images.end())
		{
			if (igd::ImageButtonText(igd::textures.images[w.icon_name]->GetDescriptorSet(), igd::textures.images[w.icon_name]->GetSize() / 2, w.desc!="" ? w.desc : w.name, {ImGui::GetContentRegionAvail().x, 50}))
				clicked = true;
		}
		else if (igd::ImageButtonText(igd::textures.images["widget2"]->GetDescriptorSet(), igd::textures.images["widget2"]->GetSize() / 2, w.desc != "" ? w.desc : w.name, { ImGui::GetContentRegionAvail().x, 50}))
			clicked = true;

		if (clicked)
			igd::active_workspace->load(w.file);
	//	ImGui::TableNextColumn();
	//		if (ImGui::Selectable(w.desc.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
	//			igd::active_workspace->load(w.file);
			
			

		if (ImGui::BeginPopupContextItem(name.c_str()))
		{
			if (ImGui::MenuItem("Add"))
			{
				igd::active_workspace->load(w.file);
			}
			if (ImGui::MenuItem("Delete"))
			{
				igd::dialogs->Confirmation("Delete", "Are you sure you wish to delete " + w.file.filename().string(), "", [w, this](bool conf) {
					if (conf)
					{
						std::filesystem::remove(w.file);
						//remove from map
						widgets.erase(w.file);
					}
					});
			}
			ImGui::EndPopup();
		}
		i++;
	}
	//ImGui::EndTable();
	ImGui::PopStyleColor(3);
}
void ToolBar::OnUIRender() {


	if (igd::active_workspace)
	{
		igd::active_workspace->RenderCode();
		igd::active_workspace->RenderAdd();
	}
	else
		return;
	
	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;

	igd::push_designer_theme();
	
	ImGui::Begin("ToolBar");
	ImGui::GetCurrentWindow()->DockNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
	ImGui::Text("Interaction Mode");
	ImGui::Separator();
	ImGui::PushItemWidth(140);
	if (ImGui::BeginCombo("##", igd::active_workspace->interaction_mode == InteractionMode::designer ? "Designer" : "User"))
	{
		if (ImGui::Selectable("Designer"))
		{
			igd::active_workspace->interaction_mode = InteractionMode::designer;
		}
		if (ImGui::Selectable("User"))
		{
			igd::active_workspace->interaction_mode = InteractionMode::user;
		}
		ImGui::EndCombo();
	}
	ImGui::Spacing(); ImGui::Spacing();
	ImGui::Spacing(); ImGui::Spacing();
	ImGui::BeginTabBar("##Toolbar_Tabs");
	if (ImGui::BeginTabItem("Elements"))
	{
		this->RenderElements();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Widgets"))
	{
		this->RenderCustomWidgets();
		ImGui::EndTabItem();
	}
	
	if (ImGui::IsAnyItemHovered())
	{
		g.MouseCursor = ImGuiMouseCursor_Hand;
	}
	ImGui::EndTabBar();
	ImGui::End();
	igd::pop_designer_theme();
}