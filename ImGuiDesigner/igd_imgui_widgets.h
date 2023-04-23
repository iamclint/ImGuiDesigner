#pragma once
#include "imgui.h"
#include <string>
namespace ImGui
{
    void ImageRounded(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col, const float& rounding);
    void SetDefaultText(std::string text);
};

