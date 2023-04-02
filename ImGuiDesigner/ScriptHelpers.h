#pragma once
#include <string>
#include <sstream>
#include "imgui.h"
#include "ImGuiElement.h"
namespace igd
{
	namespace script
	{
		std::string GetFloatString(float val);
		std::string GetVec2String(ImVec2& v);
		std::string GetVec4String(ImVec4& v);
		std::string GetSizeScript(ImGuiElement* ele);
		std::string GetWidthScript(ImGuiElement* ele);
		bool IsFlagGroup(std::pair<int, std::string> current_flag, ImGuiElement* ele);
		std::string BuildFlagString(ImGuiElement* ele);
	}
}