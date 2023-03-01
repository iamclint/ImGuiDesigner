#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"

class Notifications : public Walnut::Layer
{
public:
	Notifications() : show_generic(false), title(""), message(""), icon_path(""), button_text(""), show_confirm(false) {}
	void GenericNotification(std::string title, std::string message, std::string icon_path="", std::string button_text="Ok", std::function<void()> callback=[](){});
	void Confirmation(std::string title, std::string message, std::string icon_path, std::function<void(bool)> callback);
	void SaveFile(std::function<void(std::string)> callback);
	void OpenFile(std::function<void(std::string)> callback);
	bool IsShowing();
	virtual void OnUIRender() override;
private:
	void generic();
	void confirm();
	//generic notify stuff
	bool show_generic;
	bool show_confirm;
	std::function<void()> callback;
	std::function<void(bool)> callback_confirmation;
	std::string title;
	std::string message;
	std::string icon_path;
	std::string button_text;
};