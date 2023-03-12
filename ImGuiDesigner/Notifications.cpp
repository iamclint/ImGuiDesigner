#include "Notifications.h"
#include <Windows.h>
#include "Walnut/Application.h"
#include "ImGuiDesigner.h"
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
void Notifications::SaveFile(std::function<void(std::string)> callback)
{
	std::thread f = std::thread([callback]()
	{
		OPENFILENAMEA ofn;
		char szFileName[MAX_PATH] = "";

		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "ImGuiDesigner Files (*.igd)\0*.igd\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = szFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "igd";

		if (GetSaveFileNameA(&ofn))
		{
			callback(std::string(ofn.lpstrFile));
		}
	});
	f.detach();
}
void Notifications::OpenFile(std::function<void(std::string)> callback)
{
	std::thread f = std::thread([callback]()
		{
			OPENFILENAMEA ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "ImGuiDesigner Files (*.igd)\0*.igd\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "igd";

	if (GetOpenFileNameA(&ofn))
	{
		callback(std::string(ofn.lpstrFile));
	}
		});
	f.detach();
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
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		if (ImGui::Button("Yes", {140, 0}) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_KeyPadEnter)))
		{
			this->callback_confirmation(true);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", { 140, 0 }) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
		{
			this->callback_confirmation(false);
			ImGui::CloseCurrentPopup();
		}
		ImGui::Spacing();
		ImGui::EndPopup();
	}
}

bool Notifications::IsShowing()
{
	return this->show_generic || this->show_confirm;
}
void Notifications::OnUIRender()
{
	igd::push_designer_theme();
	generic();
	confirm();
	igd::pop_designer_theme();
}