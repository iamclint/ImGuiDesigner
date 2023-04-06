#include "Dialogs.h"
#include <Windows.h>
#include "Walnut/Application.h"
#include "ImGuiDesigner.h"
#include "misc/cpp/imgui_stdlib.h"

void Dialogs::GenericNotification(std::string title, std::string message, std::string icon_path, std::string button_text, std::function<void()> callback)
{
	this->show_generic = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->button_text = button_text;
	this->callback = callback;
}
void Dialogs::Confirmation(std::string title, std::string message, std::string icon_path, std::function<void(bool)> callback)
{
	this->show_confirm = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->callback_confirmation = callback;
}

void Dialogs::InputTextVec(std::string title, std::vector<std::string> messages, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::vector<std::string>)> callback)
{
	this->input_text_vec.clear();
	this->input_text_vec.resize(messages.size());
	this->show_inputtext_vec = true;
	this->title = title;
	this->message_vec = messages;
	this->icon_path = icon_path;
	this->callback_inputtext_vec = callback;
	this->input_buttons = buttons;
}
void Dialogs::InputText(std::string title, std::string message, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::string)> callback)
{
	this->show_inputtext = true;
	this->title = title;
	this->message = message;
	this->icon_path = icon_path;
	this->callback_inputtext = callback;
	this->input_buttons = buttons;
}
void Dialogs::SaveFile(std::function<void(std::string)> callback)
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
void Dialogs::OpenFile(std::function<void(std::string)> callback, std::string filter)
{
	std::thread f = std::thread([callback, filter]()
		{
			OPENFILENAMEA ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = filter.c_str();
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

void Dialogs::generic()
{
	if (this->show_generic)
	{
		ImGui::OpenPopup((this->title + "##generic").c_str());
		this->show_generic = false;
	}
	if (this->BeginDialog((this->title + "##generic").c_str()))
	{
		ImVec2 size = ImGui::CalcTextSize(this->message.c_str());
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
		ImGui::Text(this->message.c_str());

		if (ImGui::Button(this->button_text.c_str()) || ImGui::GetIO().KeysDown[ImGuiKey_Escape])
		{
			igd::UnPressKey(ImGuiKey_Escape);
			this->callback();
			ImGui::CloseCurrentPopup();
		}
	}
	this->EndDialog();
}
void Dialogs::confirm()
{
	if (this->show_confirm)
	{
		ImGui::OpenPopup((this->title + "##confirm").c_str());
		this->show_confirm = false;
	}

	if (this->BeginDialog((this->title + "##confirm").c_str()))
	{
			ImVec2 size = ImGui::CalcTextSize(this->message.c_str());
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
		ImGui::Text(this->message.c_str());

		ImGui::Dummy({ 0,20 });
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
	}
	this->EndDialog();
}

void Dialogs::textinput()
{
	if (this->show_inputtext)
	{
		ImGui::OpenPopup((this->title + "##input_text").c_str());
		this->show_inputtext = false;
	}
	if (this->BeginDialog((this->title + "##input_text").c_str()))
	{
		ImVec2 size = ImGui::CalcTextSize(this->message.c_str());
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
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
	}
	this->EndDialog();
}
bool Dialogs::BeginDialog(const char* title)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, { 20, 20 });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	is_open = ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
	if (is_open)
	{
		ImGui::PushFont(igd::font_manager->GetFont("designer", 24)->font);
		ImVec2 size = ImGui::CalcTextSize(this->title.c_str());
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
		ImGui::Text(this->title.c_str());
		ImGui::PopFont();
		ImGui::Dummy({ 0, 20 });

	}
	return is_open;
}
void Dialogs::EndDialog()
{
	if (is_open)
		ImGui::EndPopup();
	is_open = false;
	ImGui::PopStyleVar(3);
}
void Dialogs::textinputvec()
{
	if (this->show_inputtext_vec)
	{
		ImGui::OpenPopup((this->title + "##input_text_vec").c_str());
		this->show_inputtext_vec = false;
	}
	if (this->BeginDialog((this->title + "##input_text_vec").c_str()))
	{

		bool set_focus = false;
		for (int i = 0; auto & m : this->message_vec)
		{
			ImVec2 size = ImGui::CalcTextSize(m.c_str());
			//ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
			//ImGui::Text(m.c_str());
			if (ImGui::IsWindowAppearing() && !set_focus)
			{
				set_focus = true;
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImVec2 pos = ImGui::GetCursorPos();
			ImGui::InputText(("##input_text_text " + std::to_string(i)).c_str(), &this->input_text_vec[i]);
			//ImGui::Text(m.c_str());
			if (this->input_text_vec[i].length()==0)
				ImGui::GetWindowDrawList()->AddText({ImGui::GetWindowPos().x+pos.x+5, ImGui::GetWindowPos().y + pos.y + (size.y/2)-(GImGui->Style.FramePadding.y/2)}, ImGui::GetColorU32(ImGuiCol_Text), m.c_str());
			ImGui::Dummy({ 0, 10 });
			i++;
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		
		if (ImGui::Button(input_buttons.first.c_str(), { 140, 0 }) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);
			
			this->callback_inputtext_vec(true, this->input_text_vec);
			input_text_vec.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(input_buttons.second.c_str(), { 140, 0 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			igd::UnPressKey(ImGuiKey_Escape);
			this->callback_inputtext_vec(false, this->input_text_vec);
			input_text_vec.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::Spacing();
	}
	this->EndDialog();
}

bool Dialogs::IsShowing()
{
	return this->show_generic || this->show_confirm;
}
void Dialogs::OnUIRender()
{
	igd::push_designer_theme();
	generic();
	confirm();
	textinput();
	textinputvec();
	igd::pop_designer_theme();
}