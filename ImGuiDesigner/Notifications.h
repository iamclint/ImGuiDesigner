#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"

class Notifications : public Walnut::Layer
{
public:
	Notifications() : show(false), title(""), message(""), icon_path(""), button_text("") {}
	void GenericNotification(std::string title, std::string message, std::string icon_path, std::string button_text, std::function<void()> callback);
	virtual void OnUIRender() override;
private:
	//generic notify stuff
	bool show;
	std::function<void()> callback;
	std::string title;
	std::string message;
	std::string icon_path;
	std::string button_text;
};