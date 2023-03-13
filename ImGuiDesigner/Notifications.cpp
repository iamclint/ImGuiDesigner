#include "Notifications.h"
#include <Windows.h>
#include "Walnut/Application.h"
#include "ImGuiDesigner.h"
#include "misc/cpp/imgui_stdlib.h"
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
void Notifications::InputText(std::string title, std::string message, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::string)> callback)
{
	this->show_inputtext = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->callback_inputtext = callback;
	this->input_buttons = buttons;
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
			igd::UnPressKey(ImGuiKey_Escape);
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
		if (ImGui::Button("Yes", {140, 0}) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);
			this->callback_confirmation(true);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", { 140, 0 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			igd::UnPressKey(ImGuiKey_Escape);
			this->callback_confirmation(false);
			ImGui::CloseCurrentPopup();
		}
		ImGui::Spacing();
		ImGui::EndPopup();
	}
}
void Notifications::textinput()
{
	if (this->show_inputtext)
	{
		ImGui::OpenPopup((this->title + "##input_text").c_str());
		this->show_inputtext = false;
	}
	if (ImGui::BeginPopupModal((this->title + "##input_text").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(this->message.c_str());
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		ImGui::InputText("##input_text_text", &this->input_text);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		
		if (ImGui::Button(input_buttons.first.c_str(), { 140, 0 }) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);
			
			this->callback_inputtext(true, this->input_text);
			this->input_text = "";
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(input_buttons.second.c_str(), { 140, 0 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			igd::UnPressKey(ImGuiKey_Escape);
			this->callback_inputtext(false, this->input_text);
			this->input_text = "";
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
	textinput();
	igd::pop_designer_theme();
}