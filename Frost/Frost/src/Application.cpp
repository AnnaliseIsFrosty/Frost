#include "Application.h"
#include <SDL3/SDL.h>


namespace Frost {

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback([this](Event& e) {OnEvent(e); });

	}

	Application::~Application() {

	}

	void Application::Run() {
		while (m_Running)
		{
			std::cout << "Mouse X : " << Input::GetMouseX() << " ; Mouse Y : " << Input::GetMouseY() << std::endl;

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		//std::cout << "Frosty Event Called! Hee-Ho!" << std::endl;
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& ev) { return OnWindowClose(ev); });
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}