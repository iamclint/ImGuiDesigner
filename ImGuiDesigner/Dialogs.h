#pragma once
#include "Application.h"
#include "ImGuiElement.h"

class Dialogs : public Walnut::Layer
{
public:
	Dialogs() : show_generic(false), title(""), message(""), icon_path(""), button_text(""), show_confirm(false), show_inputtext(false), show_inputtext_vec(false), is_open(false), show_settings(false), show_savewidget(false){}
	void GenericNotification(std::string title, std::string message, std::string icon_path="", std::string button_text="Ok", std::function<void()> callback=[](){});
	void Confirmation(std::string title, std::string message, std::string icon_path, std::function<void(bool)> callback);
	void ShowSettings();
	/// <summary>
	/// Input text popup
	/// </summary>
	/// <param name="title">title</param>
	/// <param name="message">message</param>
	/// <param name="icon_path">unused currently</param>
	/// <param name="buttons">Pair of buttons first button returns true, second false</param>
	/// <param name="callback">callback function on button press</param>
	void InputText(std::string title, std::string message, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::string)> callback);
	void InputTextVec(std::string title, std::vector<std::string> messages, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::vector<std::string>)> callback);
	void SaveWidget(std::string title, std::vector<std::string> messages, std::string icon_path, std::pair<std::string, std::string> buttons, std::function<void(bool, std::vector<std::string>, std::string icon_name)> callback);
	void SaveFile(std::function<void(std::string)> callback);
	void OpenFile(std::function<void(std::string)> callback, const char* filter = "ImGuiDesigner Files (*.igd)\0*.igd\0All Files\0*.*\0\0");
	bool IsShowing();
	
	virtual void OnUIRender() override;
private:
	bool BeginDialog(const char* title);
	void EndDialog();
	void Settings();
	void generic();
	void confirm();
	void textinput();
	void textinputvec();
	void widget();
	//generic notify stuff
	bool is_open;
	bool show_generic;
	bool show_confirm;
	bool show_inputtext;
	bool show_inputtext_vec;
	bool show_savewidget;
	bool show_settings;
	std::pair<std::string, std::string> input_buttons;
	std::function<void()> callback;
	std::function<void(bool)> callback_confirmation;
	std::function<void(bool, std::string)> callback_inputtext;
	std::function<void(bool, std::vector<std::string>)> callback_inputtext_vec;
	std::function<void(bool, std::vector<std::string>, std::string)> callback_widget;
	
	std::string title;
	std::string message;
	std::string icon_path;
	std::string button_text;
	std::string input_text;
	std::vector<std::string> message_vec;
	std::vector<std::string> input_text_vec;
};