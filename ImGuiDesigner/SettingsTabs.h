#pragma once
#include <unordered_map>
#include <functional>
namespace igd
{
	extern std::unordered_map<const char*, std::function<void()>> settings_tabs;
};

