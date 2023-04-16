#include "Dialogs.h"
#include <Windows.h>
#include "Walnut/Application.h"
#include "ImGuiDesigner.h"
#include "misc/cpp/imgui_stdlib.h"
#include <iostream>
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
void Dialogs::SaveWidget(std::string title, std::vector<std::string> messages, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::vector<std::string>, std::string icon_name)> callback)
{
	this->input_text_vec.clear();
	this->input_text_vec.resize(messages.size());
	this->show_savewidget = true;
	this->title = title;
	this->message_vec = messages;
	this->icon_path = icon_path;
	this->callback_widget = callback;
	this->input_buttons = buttons;
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
		std::string f = igd::WordWrap(this->message, 70);
		ImVec2 size = ImGui::CalcTextSize(f.c_str());
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
		ImGui::Text(f.c_str());
		ImGui::Dummy({0, 20 });
		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x - GImGui->Style.FramePadding.x;
		if (ImGui::Button(this->button_text.c_str(), { width , 45 }) || ImGui::GetIO().KeysDown[ImGuiKey_Escape])
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
		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x / 2 - GImGui->Style.FramePadding.x;
		if (ImGui::Button("Yes", { width, 45}) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);
			this->callback_confirmation(true);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			igd::UnPressKey(ImGuiKey_Escape);
			this->callback_confirmation(false);
			ImGui::CloseCurrentPopup();
		}
		ImGui::Spacing();
	}
	this->EndDialog();
}

void Dialogs::ShowSettings()
{
	show_settings = true;
}

void Dialogs::Settings()
{
	if (this->show_settings)
	{
		this->title = "Settings";
		ImGui::OpenPopup("Settings");
		this->show_settings = false;
	}
	ImGui::SetNextWindowSize({ 800, 600 });
	if (this->BeginDialog("Settings"))
	{
		/*ImGui::PushFont(igd::font_manager->GetFont("designer", 36)->font);
		ImVec2 size = ImGui::CalcTextSize("Settings");
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - size.x) / 2);
		ImGui::Text("Settings");
		ImGui::PopFont();*/
		ImGui::Dummy({ 0,20 });
		if (ImGui::Checkbox("Select all selects all children", &igd::settings->bools["select_children"]))
			igd::settings->save();

		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x - GImGui->Style.FramePadding.x;
		
		ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetWindowSize().y  - (GImGui->Style.FramePadding.y*2)-60 });
		if (ImGui::Button("Ok", { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			igd::UnPressKey(ImGuiKey_Escape);
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
		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x / 2 - GImGui->Style.FramePadding.x;
		if (ImGui::Button(input_buttons.first.c_str(), { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);

			this->callback_inputtext(true, this->input_text);
			this->input_text = "";
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(input_buttons.second.c_str(), { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
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
		ImGui::PushFont(igd::font_manager->GetFont("designer", 28)->font);
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
		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x / 2 - GImGui->Style.FramePadding.x;
		if (ImGui::Button(input_buttons.first.c_str(), { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);
			
			this->callback_inputtext_vec(true, this->input_text_vec);
			input_text_vec.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(input_buttons.second.c_str(), { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
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

void Dialogs::widget()
{
	static std::string sel_texture = "";
	if (this->show_savewidget)
	{
		sel_texture = "";
		ImGui::OpenPopup((this->title + "##save_widget").c_str());
		this->show_savewidget = false;
	}
	if (this->BeginDialog((this->title + "##save_widget").c_str()))
	{
		ImGuiContext& g = *GImGui;
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
			if (this->input_text_vec[i].length() == 0)
				ImGui::GetWindowDrawList()->AddText({ ImGui::GetWindowPos().x + pos.x + 7, ImGui::GetWindowPos().y + pos.y + (size.y / 2) - (GImGui->Style.FramePadding.y / 2) }, ImGui::GetColorU32(ImGuiCol_Text), m.c_str());
			ImGui::Dummy({ 0, 10 });
			i++;
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImColor(65, 67, 74, 65).Value);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(65, 67, 74, 255).Value);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(65, 67, 74, 255).Value);
		ImGui::BeginTable("##textures", 5, ImGuiTableFlags_SizingStretchSame);
		for (auto& [name, data] : igd::textures.images)
		{
			ImGui::TableNextColumn();
			if (igd::ImageButton(data->GetDescriptorSet(), data->GetSize() / 2, { 50, 50 }, name==sel_texture ? ImColor(65, 67, 74, 255) : ImColor(0,0,0,0)))
				sel_texture = name;
		}
		ImGui::EndTable();
		ImGui::PopStyleColor(3);
		ImGui::Spacing();
		ImGui::Spacing();
		ImVec2 b_size = ImGui::GetContentRegionAvail();
		float width = b_size.x / 2 - GImGui->Style.FramePadding.x;
		if (ImGui::Button(input_buttons.first.c_str(), { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeyPadEnter))
		{
			igd::UnPressKey(ImGuiKey_Enter);
			igd::UnPressKey(ImGuiKey_KeyPadEnter);

			this->callback_widget(true, this->input_text_vec, sel_texture);
			input_text_vec.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(input_buttons.second.c_str(), { width, 45 }) || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			igd::UnPressKey(ImGuiKey_Escape);
			this->callback_widget(false, this->input_text_vec, sel_texture);
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
	widget();
	confirm();
	textinput();
	textinputvec();
	Settings();
	igd::pop_designer_theme();
}