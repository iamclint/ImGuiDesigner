#pragma once
#include <string>
#include <vector>
namespace StringAlgorithm
{
    std::vector<std::string> split(const std::string& str, const std::string& delim);
    void replaceAll(std::string& str, const std::string& oldStr, const std::string& newStr);
};

