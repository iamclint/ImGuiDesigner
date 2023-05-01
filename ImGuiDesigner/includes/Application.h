#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "imgui/imgui.h"
#include "vulkan/vulkan.h"

void check_vk_result(VkResult err);

struct GLFWwindow;

namespace Walnut {

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;
		void* Font = nullptr;
		int FontDataSize = 0;
		float FontSize = 18.0f;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();

		static Application& Get();
		
		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback) { m_MenubarCallback = menubarCallback; }
		void SetUpdateLayerStackCallback(const std::function<void()>& updateLayerstackCallback) { m_UpdateLayerStackCallback = updateLayerstackCallback; }
		
		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}
		void UpdateFonts();
		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }
		void PopLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.erase(std::remove(m_LayerStack.begin(), m_LayerStack.end(), layer), m_LayerStack.end()); layer->OnDetach(); }
		std::shared_ptr<Layer> GetLayerByPtr(Layer* layer) { for (auto& l : m_LayerStack) if (l.get() == layer) return l; return nullptr; }
		void Close();

		float GetTime();
		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

		static VkInstance GetInstance();
		static VkPhysicalDevice GetPhysicalDevice();
		static VkDevice GetDevice();

		static VkCommandBuffer GetCommandBuffer(bool begin);
		static void FlushCommandBuffer(VkCommandBuffer commandBuffer);

		static void SubmitResourceFree(std::function<void()>&& func);
	private:
		void Init();
		void Shutdown();
	private:
		ApplicationSpecification m_Specification;
		GLFWwindow* m_WindowHandle = nullptr;
		bool m_Running = false;

		float m_TimeStep = 0.0f;
		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenubarCallback;
		std::function<void()> m_UpdateLayerStackCallback;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}