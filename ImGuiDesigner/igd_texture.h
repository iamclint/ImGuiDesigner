#pragma once
#include "ImGuiElement.h"
#include <string>
#include <map>
#include "misc/cpp/imgui_stdlib.h"
#include "ImGuiDesigner.h"
#include <iostream>
#include <map>
#include "Walnut/Image.h"

namespace igd
{
	class Texture : ImGuiElement
	{
	public:

		static inline std::unordered_map<Texture*, std::vector<Texture>> undo_stack;
		static inline std::unordered_map<Texture*, std::vector<Texture>> redo_stack;
		static inline std::string json_identifier = "Texture";
		Walnut::Image* img;
		Texture() {
			v_type_id = (int)element_type::texture;
			this->v_size.type = Vec2Type::Absolute;
			this->v_size.value = { 128, 128 };
		}
		Texture(std::string path) {
			v_type_id = (int)element_type::texture;
			std::cout << "Adding new texture: " << path << std::endl;
			img = new Walnut::Image(path);
			v_path = path;
			this->v_size.type = Vec2Type::Absolute;
			this->v_size.value = { (float)img->GetWidth(), (float)img->GetHeight() };
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

		virtual void UndoLocal() override
		{
			if (undo_stack[this].size() > 1)
			{
				redo_stack[this].push_back(*this);
				if (undo_stack[this].size() > 1)
					undo_stack[this].pop_back();

				*this = undo_stack[this].back();
			}
		}
		virtual void RedoLocal() override
		{
			if (redo_stack[this].size() > 0)
			{
				*this = redo_stack[this].back();
				PushUndo();
				redo_stack[this].pop_back();
			}
		}

		virtual void PushUndoLocal() override
		{
			//keep an undo stack locally for this type
			undo_stack[this].push_back(*this);
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

		}

		std::string ScriptHead() {
			return "";
		};
		std::string ScriptInternal() {
			std::stringstream code;
			code << "//You must supply the loaded img data" << std::endl;
			code << "ImGui::Image(\"" << this->v_path << "\", " << this->GetSizeScript() << "))" << std::endl;
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
				ImGui::Image(img->GetDescriptorSet(), this->v_size.value);
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
			GenerateStylesColorsJson(j, json_identifier);
			return j;
		}

		static ImGuiElement* load(ImGuiElement* parent, nlohmann::json data)
		{
			std::cout << "Adding a Texture" << std::endl;
			igd::Texture* b = new igd::Texture(data["path"]);
			ImGuiElement* f = (ImGuiElement*)b;
			f->v_parent = parent;
			b->FromJSON(data);
			if (!parent)
				igd::active_workspace->AddNewElement((ImGuiElement*)b, true);
			else
				parent->children.push_back((ImGuiElement*)b);
			return f;
		}
	};
}