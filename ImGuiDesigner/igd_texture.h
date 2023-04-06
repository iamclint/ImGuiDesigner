#pragma once
#include "ImGuiElement.h"
#include <string>
#include <map>
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <map>
#include "Walnut/Image.h"
#include <filesystem>
namespace igd
{
	class Texture : ImGuiElement
	{
	public:
		static inline std::string json_identifier = "Texture";
		ImVec2 uv0;
		ImVec2 uv1;
		ImVec4 tint_col;
		ImVec4 border_col;
		Walnut::Image* img;
		Texture() {
			v_type_id = (int)element_type::texture;
			this->v_size.type = Vec2Type::Absolute;
			this->v_size.value = { 128, 128 };
		}
		Texture(std::string path) {
			uv0 = ImVec2(0, 0);
			uv1 = ImVec2(1, 1);
			tint_col = ImVec4(1, 1, 1, 1);
			border_col = ImVec4(0, 0, 0, 0);
			
			v_type_id = (int)element_type::texture;
			std::cout << "Adding new texture: " << path << std::endl;
			img = new Walnut::Image(path);
			v_path = path;
			this->v_size.type = Vec2Type::Absolute;
			this->v_size.value = { (float)img->GetWidth(), (float)img->GetHeight() };
			v_aspect_ratio = (float)img->GetHeight() / (float)img->GetWidth();
			if (this->v_size.value.x == 0 || this->v_size.value.y == 0)
				this->v_size.value = { 128, 128 };
			
			ImGuiContext& g = *GImGui;
			v_flags = 0;
			v_property_flags = property_flags::pos;
			v_id = ("Texture##" + RandomID()).c_str();
			v_label = "";
;
			v_can_have_children = false;
		}
		virtual ImGuiElement* Clone() override
		{
			Texture* new_element = new Texture(this->v_path);
			*new_element = *this;
			std::string new_id = this->v_id;
			if (new_id.find("##") != std::string::npos)
				new_id = new_id.substr(0, new_id.find("##") + 2);
			if (active_workspace->active_element->v_can_have_children)
				new_element->v_parent = active_workspace->active_element;
			else
				new_element->v_parent = active_workspace->active_element->v_parent;
			new_element->v_id = new_id + RandomID();
			return new_element;
		}

		//Extends the property window with the properties specific of this element
		virtual void RenderPropertiesInternal() override
		{
			igd::properties->PropertyLabel("UV0");
			ImGui::PushItemWidth(120);
			ImGui::InputFloat2("##UV0", (float*)&uv0);
			igd::properties->PropertyLabel("UV1");
			ImGui::InputFloat2("##UV1", (float*)&uv1);
			igd::properties->PropertyLabel("Tint");
			ImGui::ColorEdit4("##texture_tint", (float*)&tint_col, ImGuiColorEditFlags_NoInputs);
			igd::properties->PropertyLabel("Border");
			ImGui::ColorEdit4("##texture_border", (float*)&border_col, ImGuiColorEditFlags_NoInputs);
		}

		std::string ScriptHead() {
			return "";
		};
		std::string ScriptInternal() {
			std::stringstream code;
			code << "//You must supply the loaded img data" << std::endl;
			code << "ImGui::Image(\"" << this->v_path << "\", " << igd::script::GetSizeScript(this) << ", " << igd::script::GetVec2String(uv0) << ", " << igd::script::GetVec2String(uv1) << ", " << igd::script::GetVec4String(tint_col) << ", " << igd::script::GetVec4String(border_col) << ")" << std::endl;
			return code.str();
		};

		std::string ScriptFoot() { return ""; };

		virtual std::string RenderHead(bool script_only) override
		{
			ImGuiContext& g = *GImGui;
			return "";
		}

		virtual std::string RenderInternal(bool script_only) override
		{
			if (script_only)
				return ScriptInternal();
			if (!img->GetDescriptorSet())
				ImGui::Dummy(this->v_size.value);
			else
			{
				ImGui::Image(img->GetDescriptorSet(), this->GetSize(), uv0, uv1, tint_col, border_col);
			}
			return ScriptInternal();
		}

		virtual std::string RenderFoot(bool script_only) override
		{
			return "";
		}
		virtual void FromJSON(nlohmann::json data) override
		{
			v_path=data["path"];
			StylesColorsFromJson(data);
		}
		virtual nlohmann::json GetJson() override
		{
			nlohmann::json j;
			j["path"] = v_path;
			j["uv0"] = { { "x", uv0.x }, {"y", uv0.y} };
			j["uv1"] = { { "x", uv1.x }, {"y", uv1.y} };
			j["tint_col"] = ColorToJson(tint_col);
			j["border_col"] = ColorToJson(border_col);
			GenerateStylesColorsJson(j, json_identifier);
			return j;
		}

		static ImGuiElement* load(ImGuiElement* parent, nlohmann::json data)
		{
			//check if file exists using filesystem
			if (!std::filesystem::exists(data["path"]))
			{
				igd::dialogs->GenericNotification("Error loading texture", "The texture file " + std::string(data["path"]) + " does not exist");
				return nullptr;
			}
			igd::Texture* b = new igd::Texture(data["path"]);
			ImGuiElement* f = (ImGuiElement*)b;
			f->v_parent = parent;
			b->FromJSON(data);

			b->uv0 = { data["uv0"]["x"],data["uv0"]["y"] };
			b->uv1 = { data["uv1"]["x"],data["uv1"]["y"] };
			b->tint_col = b->JsonToColor(data["tint_col"]);
			b->tint_col = b->JsonToColor(data["border_col"]);
			if (!parent)
				igd::active_workspace->AddNewElement((ImGuiElement*)b, true);
			else
				parent->children.push_back((ImGuiElement*)b);
			return f;
		}
	};
}