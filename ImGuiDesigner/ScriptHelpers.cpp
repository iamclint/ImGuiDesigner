#pragma once
#include "ScriptHelpers.h"

namespace igd
{
	namespace script
	{
		std::string GetFloatString(float val)
		{
			std::stringstream ss;
			if (val == (int)val)
				ss << val << ".f";
			else
				ss << val << "f";
			return ss.str();
		}
		std::string GetVec2String(ImVec2& v)
		{
			std::stringstream ss;
			ss << "{" << GetFloatString(v.x) << ", " << GetFloatString(v.y) << "}";
			return ss.str();
		}
		std::string GetVec4String(ImVec4& v)
		{
			std::stringstream ss;
			ss << "{" << GetFloatString(v.x) << ", " << GetFloatString(v.y) << ", " << GetFloatString(v.z) << ", " << GetFloatString(v.w) << "}";
			return ss.str();
		}
		std::string GetSizeScript(ImGuiElement* ele)
		{
			std::stringstream c;
			if (ele->v_size.type == Vec2Type::Absolute)
				c << "{" << GetFloatString(ele->v_size.value.x) << "," << GetFloatString(ele->v_size.value.y) << "}";
			else if (ele->v_size.type == Vec2Type::Relative)
				c << "{ (" << ele->ContentRegionString << ".x * " << GetFloatString(ele->v_size.value.x / 100) << ") - GImGui->Style.FramePadding.x, (" << ele->ContentRegionString << ".y * " << GetFloatString(ele->v_size.value.y / 100) << ") - GImGui->Style.FramePadding.y }";
			return c.str();
		}

		std::string GetWidthScript(ImGuiElement* ele)
		{
			std::stringstream c;
			c << "ImGui::SetNextItemWidth(";
			if (ele->v_size.type == Vec2Type::Absolute)
				c << GetFloatString(ele->v_size.value.x);
			else if (ele->v_size.type == Vec2Type::Relative)
				c << "(" << ele->ContentRegionString << ".x * " << GetFloatString(ele->v_size.value.x / 100) << ") - GImGui->Style.FramePadding.x";
			c << ");";
			return c.str();
		}

		bool IsFlagGroup(std::pair<int, std::string> current_flag, ImGuiElement* ele)
		{
			for (auto& [flag, str] : ele->v_custom_flags)
			{
				bool any_on = (flag & current_flag.first) != 0;
				bool all_on = (ele->v_flags & flag) == flag;
				if (flag == current_flag.first)
					continue;
				if (any_on && all_on)
					return true;
			}
			return false;
		}

		std::string BuildFlagString(ImGuiElement* ele)
		{
			std::stringstream ss;
			for (auto& [flag, str] : ele->v_custom_flags)
			{
				bool any_on = (flag & ele->v_flags) != 0;
				bool all_on = (ele->v_flags & flag) == flag;
				bool is_group_on = (ele->v_custom_flag_groups[flag] && all_on);
				if ((flag & ele->v_flags) && (!IsFlagGroup({ flag,str }, ele) || is_group_on))
					ss << str << " | ";
			}
			if (ss.str().length() > 0)
				return ss.str().substr(0, ss.str().length() - 3);
			else
				return "0";
		}
	}
}
