#pragma once
#include <functional>
#include <unordered_map>
#include "igd_button.h"
#include "igd_childwindow.h"
#include "igd_text.h"
namespace igd
{
	static inline std::unordered_map<std::string, std::function<ImGuiElement*(ImGuiElement*, nlohmann::json)>> element_load_functions = {
		{igd::Button::json_identifier, igd::Button::load },
		{igd::ChildWindow::json_identifier, igd::ChildWindow::load },
		{igd::Text::json_identifier, igd::Text::load }
	};
}
