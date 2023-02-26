#include "Notifications.h"
void Notifications::GenericNotification(std::string title, std::string message, std::string icon_path, std::string button_text, std::function<void()> callback)
{
	this->show_generic = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->button_text = button_text;
	this->callback = callback;
}
void Notifications::Confirmation(std::string title, std::string message, std::string icon_path, std::function<void(bool)> callback)
{
	this->show_confirm = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->callback_confirmation = callback;
}


void Notifications::generic()
{
	if (this->show_generic)
	{
		ImGui::OpenPopup((this->title + "##generic").c_str());
		this->show_generic = false;
	}
	if (ImGui::BeginPopupModal((this->title + "##generic").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(this->message.c_str());

		if (ImGui::Button(this->button_text.c_str()) || ImGui::GetIO().KeysDown[ImGuiKey_Escape])
		{
			ImGui::GetIO().KeysDown[ImGuiKey_Escape] = false;
			this->callback();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void Notifications::confirm()
{
	if (this->show_confirm)
	{
		ImGui::OpenPopup((this->title + "##confirm").c_str());
		this->show_confirm = false;
	}
	if (ImGui::BeginPopupModal((this->title + "##confirm").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(this->message.c_str());

		if (ImGui::Button("Yes") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
		{
			this->callback_confirmation(true);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
		{
			this->callback_confirmation(false);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

bool Notifications::IsShowing()
{
	return this->show_generic || this->show_confirm;
}
void Notifications::OnUIRender()
{
	generic();
	confirm();
}