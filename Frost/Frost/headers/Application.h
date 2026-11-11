#pragma once
#include "FrostIncludes.h"
#include "Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Input.h"
#include "Renderer.h"

namespace Frost {
	class Application {
	public: 
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& event);
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer> m_Renderer;
		bool m_Running = true;
	};

	// To be defined in client
	Application* CreateApplication();
}