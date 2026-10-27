#pragma once
#include "FrostIncludes.h"
#include "Window.h"

namespace Frost {
	class Application {
	public: 
		Application();
		virtual ~Application();

		void Run();

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined in client
	Application* CreateApplication();
}