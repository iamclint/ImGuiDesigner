#include "igd_imgui_widgets.h"
#include "imgui.h"
#include "imgui_internal.h"
namespace ImGui
{
    void ImageRounded(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col, const float& rounding)
    {
        
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2, 2);
        ItemSize(bb);
        if (!ItemAdd(bb, 0))
            return;

        if (border_col.w > 0.0f)
        {
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
            window->DrawList->AddImageRounded(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col), rounding);
        }
        else
        {
            window->DrawList->AddImageRounded(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col), rounding);
        }
    }
}