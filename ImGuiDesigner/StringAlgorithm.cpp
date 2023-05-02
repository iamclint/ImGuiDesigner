#include "StringAlgorithm.h"
namespace StringAlgorithm
{
    std::vector<std::string> split(const std::string& str, const std::string& delim) {
        std::vector<std::string> tokens;

        size_t pos = 0;
        size_t len = str.length();
        size_t delimLen = delim.length();

        while (pos < len) {
            size_t found = str.find(delim, pos);
            if (found == std::string::npos) {
                tokens.push_back(str.substr(pos));
                break;
            }
            tokens.push_back(str.substr(pos, found - pos));
            pos = found + delimLen;
        }

        return tokens;
    }

    void replaceAll(std::string& str, const std::string& oldStr, const std::string& newStr) {
        size_t pos = 0;
        while ((pos = str.find(oldStr, pos)) != std::string::npos) {
            str.replace(pos, oldStr.length(), newStr);
            pos += newStr.length();
        }
    }
}