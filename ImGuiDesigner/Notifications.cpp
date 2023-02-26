#include "Notifications.h"
void Notifications::GenericNotification(std::string title, std::string message, std::string icon_path, std::string button_text, std::function<void()> callback)
{
	this->show = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->button_text = button_text;
	this->callback = callback;
}

void Notifications::OnUIRender()
{
	if (this->show)
	{
		ImGui::OpenPopup(this->title.c_str());
		this->show = false;
	}
	if (ImGui::BeginPopupModal(this->title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
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