#pragma once
#include "Walnut/Application.h"
#include "ImGuiElement.h"
#include <vector>
class WorkSpace : public Walnut::Layer
{
public:
	
	std::vector<ImGuiElement*> elements;
	std::vector<ImGuiElement*> elements_buffer;
	std::vector<ImGuiElement*> undo_stack;
	std::vector<ImGuiElement*> redo_stack;
	std::stringstream code;
	void KeyBinds();
	virtual void OnUIRender() override;
	void PushUndo(ImGuiElement* ele);
	void Styles();
	void Colors();
	WorkSpace();
	~WorkSpace();
	std::string id;
	bool is_open;



	float v_ImGuiStyleVar_DisabledAlpha;       // float     DisabledAlpha
	ImVec2 v_ImGuiStyleVar_WindowPadding;       // ImVec2    WindowPadding
	float v_ImGuiStyleVar_WindowRounding;      // float     WindowRounding
	float v_ImGuiStyleVar_WindowBorderSize;    // float     WindowBorderSize
	ImVec2 v_ImGuiStyleVar_WindowMinSize;       // ImVec2    WindowMinSize
	ImVec2 v_ImGuiStyleVar_WindowTitleAlign;    // ImVec2    WindowTitleAlign
	float v_ImGuiStyleVar_ChildRounding;       // float     ChildRounding
	float v_ImGuiStyleVar_ChildBorderSize;     // float     ChildBorderSize
	float v_ImGuiStyleVar_PopupRounding;       // float     PopupRounding
	float v_ImGuiStyleVar_PopupBorderSize;     // float     PopupBorderSize
	ImVec2 v_ImGuiStyleVar_FramePadding;        // ImVec2    FramePadding
	float v_ImGuiStyleVar_FrameRounding;       // float     FrameRounding
	float v_ImGuiStyleVar_FrameBorderSize;     // float     FrameBorderSize
	ImVec2 v_ImGuiStyleVar_ItemSpacing;         // ImVec2    ItemSpacing
	ImVec2 v_ImGuiStyleVar_ItemInnerSpacing;    // ImVec2    ItemInnerSpacing
	float v_ImGuiStyleVar_IndentSpacing;       // float     IndentSpacing
	ImVec2 v_ImGuiStyleVar_CellPadding;         // ImVec2    CellPadding
	float v_ImGuiStyleVar_ScrollbarSize;       // float     ScrollbarSize
	float v_ImGuiStyleVar_ScrollbarRounding;   // float     ScrollbarRounding
	float v_ImGuiStyleVar_GrabMinSize;         // float     GrabMinSize
	float v_ImGuiStyleVar_GrabRounding;        // float     GrabRounding
	float v_ImGuiStyleVar_TabRounding;         // float     TabRounding
	ImVec2 v_ImGuiStyleVar_ButtonTextAlign;     // ImVec2    ButtonTextAlign
	ImVec2 v_ImGuiStyleVar_SelectableTextAlign; // ImVec2    SelectableTextAlign
	float v_ImGuiStyleVar_LayoutAlign;         // float     LayoutAlign

};

