#include "Application.h"
#include <SDL3/SDL.h>


namespace Frost {

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application() {

	}

	void Application::Run() {
		while (m_Running)
		{
			m_Window->OnUpdate();
		}
	}

}