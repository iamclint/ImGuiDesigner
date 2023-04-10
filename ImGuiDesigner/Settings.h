#pragma once
#include <unordered_map>
#include <string>
class Settings
{
public:
	std::unordered_map<std::string, bool> bools;
	Settings() : bools{} {};
	void load();
	void save();
};

