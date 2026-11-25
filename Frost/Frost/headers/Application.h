#pragma once
#include "FrostIncludes.h"
#include "Window.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Input.h"
#include "Renderer.h"
#include "Timer.h"

namespace Frost {
	class Application {
	public: 
		Application();
		virtual ~Application();

		void SetApp(Application* app) { m_App = app; };

		virtual  void Run();

		void OnEvent(Event& event);

	protected:
		Application* m_App;
		Timer m_Timer;
		std::unique_ptr<Window> m_Window;
		std::unique_ptr<Renderer> m_Renderer;

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		bool m_Running = true;
	};

	// To be defined in client
	Application* CreateApplication();
}