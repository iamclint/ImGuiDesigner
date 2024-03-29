#pragma once
#include "ImGuiElement.h"
#include <string>
#include <vector>
#include "Workspace.h"

namespace igd
{
	class Table : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "table";
		int columns;
		bool use_headers;
		float inner_width;
		int freeze_cols_rows[2];
		std::vector<std::string> headers;
		Table() {
			v_icon = igd::textures.images["menu"];
			v_tooltip = "table";
			v_type_id = (int)element_type::table;
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos | property_flags::disabled;
			v_size = ImVec2(0, 0);
			freeze_cols_rows[0] = 0;
			freeze_cols_rows[1] = 0;
			v_id = ("table##" + RandomID()).c_str();
			v_label = "";
			v_border = true;
			v_can_have_children = true;
			columns = 3;
			headers.resize(3);
			use_headers = true;
			inner_width = 0;
			v_requires_open = true;
			//available table flags
			v_custom_flags[ImGuiTableFlags_Borders] = "ImGuiTableFlags_Borders";
			v_custom_flags[ImGuiTableFlags_BordersH] = "ImGuiTableFlags_BordersH";
			v_custom_flags[ImGuiTableFlags_BordersInner] = "ImGuiTableFlags_BordersInner";
			v_custom_flags[ImGuiTableFlags_BordersInnerH] = "ImGuiTableFlags_BordersInnerH";
			v_custom_flags[ImGuiTableFlags_BordersInnerV] = "ImGuiTableFlags_BordersInnerV";
			v_custom_flags[ImGuiTableFlags_BordersOuter] = "ImGuiTableFlags_BordersOuter";
			v_custom_flags[ImGuiTableFlags_BordersOuterH] = "ImGuiTableFlags_BordersOuterH";
			v_custom_flags[ImGuiTableFlags_BordersOuterV] = "ImGuiTableFlags_BordersOuterV";
			v_custom_flags[ImGuiTableFlags_ContextMenuInBody] = "ImGuiTableFlags_ContextMenuInBody";
			v_custom_flags[ImGuiTableFlags_Hideable] = "ImGuiTableFlags_Hideable";
			v_custom_flags[ImGuiTableFlags_NoBordersInBody] = "ImGuiTableFlags_NoBordersInBody";
			v_custom_flags[ImGuiTableFlags_NoBordersInBodyUntilResize] = "ImGuiTableFlags_NoBordersInBodyUntilResize";
			v_custom_flags[ImGuiTableFlags_NoClip] = "ImGuiTableFlags_NoClip";
			v_custom_flags[ImGuiTableFlags_NoHostExtendX] = "ImGuiTableFlags_NoHostExtendX";
			v_custom_flags[ImGuiTableFlags_NoHostExtendY] = "ImGuiTableFlags_NoHostExtendY";
			v_custom_flags[ImGuiTableFlags_NoKeepColumnsVisible] = "ImGuiTableFlags_NoKeepColumnsVisible";
			v_custom_flags[ImGuiTableFlags_NoPadInnerX] = "ImGuiTableFlags_NoPadInnerX";
			v_custom_flags[ImGuiTableFlags_NoPadOuterX] = "ImGuiTableFlags_NoPadOuterX";
			v_custom_flags[ImGuiTableFlags_NoSavedSettings] = "ImGuiTableFlags_NoSavedSettings";
			v_custom_flags[ImGuiTableFlags_PadOuterX] = "ImGuiTableFlags_PadOuterX";
			v_custom_flags[ImGuiTableFlags_PreciseWidths] = "ImGuiTableFlags_PreciseWidths";
			v_custom_flags[ImGuiTableFlags_Reorderable] = "ImGuiTableFlags_Reorderable";
			v_custom_flags[ImGuiTableFlags_Resizable] = "ImGuiTableFlags_Resizable";
			v_custom_flags[ImGuiTableFlags_RowBg] = "ImGuiTableFlags_RowBg";
			v_custom_flags[ImGuiTableFlags_ScrollX] = "ImGuiTableFlags_ScrollX";
			v_custom_flags[ImGuiTableFlags_ScrollY] = "ImGuiTableFlags_ScrollY";
			v_custom_flags[ImGuiTableFlags_SizingFixedFit] = "ImGuiTableFlags_SizingFixedFit";
			v_custom_flags[ImGuiTableFlags_SizingFixedSame] = "ImGuiTableFlags_SizingFixedSame";
			v_custom_flags[ImGuiTableFlags_SizingMask_] = "ImGuiTableFlags_SizingMask";
			v_custom_flags[ImGuiTableFlags_SizingStretchProp] = "ImGuiTableFlags_SizingStretchProp";
			v_custom_flags[ImGuiTableFlags_SizingStretchSame] = "ImGuiTableFlags_SizingStretchSame";
			v_custom_flags[ImGuiTableFlags_Sortable] = "ImGuiTableFlags_Sortable";
			v_custom_flags[ImGuiTableFlags_SortMulti] = "ImGuiTableFlags_SortMulti";
			v_custom_flags[ImGuiTableFlags_SortTristate] = "ImGuiTableFlags_SortTristate";

			v_custom_flag_groups[ImGuiTableFlags_SizingMask_] = true;
			v_custom_flag_groups[ImGuiTableFlags_Borders] = true;
			v_custom_flag_groups[ImGuiTableFlags_BordersH] = true;
			v_custom_flag_groups[ImGuiTableFlags_BordersV] = true;
			v_custom_flag_groups[ImGuiTableFlags_BordersInner] = true;
			v_custom_flag_groups[ImGuiTableFlags_BordersOuter] = true;


			v_colors[ImGuiCol_Text] = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			v_colors[ImGuiCol_TableBorderLight] = ImGui::GetStyleColorVec4(ImGuiCol_TableBorderLight);
			v_colors[ImGuiCol_TableBorderStrong] = ImGui::GetStyleColorVec4(ImGuiCol_TableBorderStrong);
			v_colors[ImGuiCol_TableHeaderBg] = ImGui::GetStyleColorVec4(ImGuiCol_TableHeaderBg);
			v_colors[ImGuiCol_TableRowBg] = ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg);
			v_colors[ImGuiCol_TableRowBgAlt] = ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt);


			v_colors[ImGuiCol_Button] = ImGui::GetStyleColorVec4(ImGuiCol_Button);
			v_colors[ImGuiCol_ButtonHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
			v_colors[ImGuiCol_ButtonActive] = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			v_colors[ImGuiCol_Border] = ImGui::GetStyleColorVec4(ImGuiCol_Border);
			v_colors[ImGuiCol_BorderShadow] = ImGui::GetStyleColorVec4(ImGuiCol_BorderShadow);
			v_colors[ImGuiCol_ChildBg] = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
			v_colors[ImGuiCol_CheckMark] = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
			v_colors[ImGuiCol_FrameBg] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
			v_colors[ImGuiCol_FrameBgActive] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive);
			v_colors[ImGuiCol_FrameBgHovered] = ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered);
			v_colors[ImGuiCol_Header] = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			v_colors[ImGuiCol_HeaderActive] = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
			v_colors[ImGuiCol_HeaderHovered] = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			v_colors[ImGuiCol_NavHighlight] = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
			v_colors[ImGuiCol_PlotHistogram] = ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram);
			v_colors[ImGuiCol_PlotHistogramHovered] = ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogramHovered);
			v_colors[ImGuiCol_PlotLines] = ImGui::GetStyleColorVec4(ImGuiCol_PlotLines);
			v_colors[ImGuiCol_PlotLinesHovered] = ImGui::GetStyleColorVec4(ImGuiCol_PlotLinesHovered);
			v_colors[ImGuiCol_ScrollbarBg] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarBg);
			v_colors[ImGuiCol_ScrollbarGrab] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrab);
			v_colors[ImGuiCol_ScrollbarGrabActive] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabActive);
			v_colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabHovered);
			v_colors[ImGuiCol_SliderGrab] = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab);
			v_colors[ImGuiCol_SliderGrabActive] = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrabActive);
			v_colors[ImGuiCol_Tab] = ImGui::GetStyleColorVec4(ImGuiCol_Tab);
			v_colors[ImGuiCol_TabActive] = ImGui::GetStyleColorVec4(ImGuiCol_TabActive);
			v_colors[ImGuiCol_TabHovered] = ImGui::GetStyleColorVec4(ImGuiCol_TabHovered);
			v_colors[ImGuiCol_TabUnfocused] = ImGui::GetStyleColorVec4(ImGuiCol_TabUnfocused);
			v_colors[ImGuiCol_TabUnfocusedActive] = ImGui::GetStyleColorVec4(ImGuiCol_TabUnfocusedActive);
			v_colors[ImGuiCol_ResizeGrip] = ImGui::GetStyleColorVec4(ImGuiCol_ResizeGrip);
			v_colors[ImGuiCol_ResizeGripActive] = ImGui::GetStyleColorVec4(ImGuiCol_ResizeGripActive);
			v_colors[ImGuiCol_ResizeGripHovered] = ImGui::GetStyleColorVec4(ImGuiCol_ResizeGripHovered);
			v_colors[ImGuiCol_Separator] = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
			v_colors[ImGuiCol_SeparatorActive] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive);
			v_colors[ImGuiCol_SeparatorHovered] = ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered);
			v_colors[ImGuiCol_TextSelectedBg] = ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg);
			v_colors[ImGuiCol_TextDisabled] = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
			v_colors[ImGuiCol_DragDropTarget] = ImGui::GetStyleColorVec4(ImGuiCol_DragDropTarget);
			v_colors[ImGuiCol_NavWindowingHighlight] = ImGui::GetStyleColorVec4(ImGuiCol_NavWindowingHighlight);
			v_colors[ImGuiCol_NavWindowingDimBg] = ImGui::GetStyleColorVec4(ImGuiCol_NavWindowingDimBg);

			v_styles[ImGuiStyleVar_LayoutAlign] = g.Style.LayoutAlign;
			v_styles[ImGuiStyleVar_CellPadding] = g.Style.CellPadding;
			v_styles[ImGuiStyleVar_ChildBorderSize] = g.Style.ChildBorderSize;
			v_styles[ImGuiStyleVar_ChildRounding] = g.Style.ChildRounding;
			v_styles[ImGuiStyleVar_ButtonTextAlign] = g.Style.ButtonTextAlign;
			v_styles[ImGuiStyleVar_DisabledAlpha] = g.Style.DisabledAlpha;
			v_styles[ImGuiStyleVar_FrameBorderSize] = g.Style.FrameBorderSize;
			v_styles[ImGuiStyleVar_FramePadding] = g.Style.FramePadding;
			v_styles[ImGuiStyleVar_FrameRounding] = g.Style.FrameRounding;
			v_styles[ImGuiStyleVar_GrabMinSize] = g.Style.GrabMinSize;
			v_styles[ImGuiStyleVar_GrabRounding] = g.Style.GrabRounding;
			v_styles[ImGuiStyleVar_ItemInnerSpacing] = g.Style.ItemInnerSpacing;
			v_styles[ImGuiStyleVar_ItemSpacing] = g.Style.ItemSpacing;
			v_styles[ImGuiStyleVar_ScrollbarRounding] = g.Style.ScrollbarRounding;
			v_styles[ImGuiStyleVar_ScrollbarSize] = g.Style.ScrollbarSize;
			v_styles[ImGuiStyleVar_SelectableTextAlign] = g.Style.SelectableTextAlign;
		}

		virtual ImGuiElement* Clone() override
		{
			Table* new_element = new Table();
			*new_element = *this;
			std::string new_id = this->v_id;
			if (new_id.find("##") != std::string::npos)
				new_id = new_id.substr(0, new_id.find("##") + 2);

			if (igd::active_workspace->GetSingleSelection()->v_can_have_children)
				new_element->v_parent = igd::active_workspace->GetSingleSelection();
			else
				new_element->v_parent = igd::active_workspace->GetSingleSelection()->v_parent;

			new_element->v_id = new_id + RandomID();
			new_element->children.clear();
			for (auto& child : this->children)
			{
				ImGuiElement* n = child->Clone();
				n->v_parent = new_element;
				new_element->children.push_back(n);
			}
			return new_element;
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{
			igd::properties->PropertyLabel("Use Headers");
			ImGui::Checkbox("##property_table_use_headers", &use_headers);
			igd::properties->PropertyLabel("Freeze Cols/Rows");
			ImGui::InputInt2("##property_table_freeze", freeze_cols_rows);
			igd::properties->PropertyLabel("Columns");
			if (ImGui::InputInt("##property_table_columns", &columns))
				headers.resize(columns);
			if (use_headers)
			{
				for (int i = 0; i < headers.size(); i++)
				{
					igd::properties->PropertyLabel(("Header " + std::to_string(i)).c_str());
					ImGui::InputText(("##property_table_header_" + std::to_string(i)).c_str(), &headers[i], ImGuiInputTextFlags_EnterReturnsTrue);
				}
			}
		}

		std::string ScriptHead()
		{
			std::stringstream code_out;
			code_out << "if (ImGui::BeginTable(\"" << v_id << "\"," << columns << ", " << igd::script::BuildFlagString(this) << ", " << igd::script::GetSizeScript(this) << ", " << igd::script::GetFloatString(inner_width) << "))";

			return code_out.str();
		}

		std::string ScriptFoot()
		{
			return "ImGui::EndTable();";
		}

		virtual std::string RenderHead(bool script_only) override
		{
			if (v_id == "")
				return "";
			if (script_only)
				return ScriptHead();
			ImGuiContext& g = *GImGui;

			if (columns < 1)
				columns = 1;

			v_is_open = ImGui::BeginTable(v_id.c_str(), columns, v_flags, this->GetSize(), inner_width);
			if (v_is_open)
			{
				if (freeze_cols_rows[0] != 0 || freeze_cols_rows[1] != 0)
					ImGui::TableSetupScrollFreeze(freeze_cols_rows[0], freeze_cols_rows[1]);

				if (use_headers)
				{
					for (auto& e : headers)
					{
						ImGui::TableSetupColumn(e.c_str());
					}
					ImGui::TableHeadersRow();
				}
			}
			return ScriptHead();
		}
		virtual std::string RenderInternal(bool script_only) override
		{
			std::stringstream code_out;
			if (freeze_cols_rows[0] != 0 || freeze_cols_rows[1] != 0)
				code_out << "ImGui::TableSetupScrollFreeze(" << freeze_cols_rows[0] << ", " << freeze_cols_rows[1] << ");" << std::endl;

			if (use_headers)
			{
				for (auto& e : headers)
				{
					code_out << "ImGui::TableSetupColumn(\"" << e << "\");" << std::endl;;
				}
				code_out << "ImGui::TableHeadersRow();";
			}
			//iterate all children handled by imguielement cpp
			return code_out.str();
		}
		virtual std::string RenderFoot(bool script_only) override
		{
			if (v_id == "")
				return "";
			if (v_is_open && !script_only)
				ImGui::EndTable();
			return ScriptFoot();
		}
		virtual void FromJSON(nlohmann::json data) override
		{
			StylesColorsFromJson(data);
			columns = data["columns"];
			//headers.resize(data["column_array"].size());
			headers.clear();
			for (auto& e : data["column_array"])
			{
				headers.push_back(e);
			}
			freeze_cols_rows[0] = data["freeze_cols_rows"]["x"];
			freeze_cols_rows[1] = data["freeze_cols_rows"]["y"];
			inner_width = data["inner_width"];
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			GenerateStylesColorsJson(j, json_identifier);
			j["columns"] = columns;
			j["column_array"] = nlohmann::json::array();
			for (auto& e : headers)
				j["column_array"].push_back(e);
			j["freeze_cols_rows"] = { { "x", freeze_cols_rows[0] }, {"y", freeze_cols_rows[1]} };
			j["inner_width"] = inner_width;
			return j;
		}

		static ImGuiElement* load(ImGuiElement* parent, nlohmann::json data)
		{
			ImGuiElement* new_parent = nullptr;
			std::cout << "table found" << std::endl;
			igd::Table* b = new igd::Table();
			b->FromJSON(data);
			new_parent = (ImGuiElement*)b;
			if (parent)
				parent->children.push_back((ImGuiElement*)b);
			else
				igd::active_workspace->AddNewElement((ImGuiElement*)b, true);
			new_parent->v_parent = parent;
			return new_parent;
		}
	};
}