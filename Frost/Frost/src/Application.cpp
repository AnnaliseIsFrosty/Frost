#include "Application.h"
#include <SDL3/SDL.h>


namespace Frost {

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback([this](Event& e) {OnEvent(e); });
		

		m_Renderer = std::unique_ptr<Renderer>(Renderer::Create(m_Window->GetWindow()));

	}

	Application::~Application() {

	}

	void Application::Run() {
		while (m_Running)
		{
			//std::cout << "Mouse X : " << Input::GetMouseX() << " ; Mouse Y : " << Input::GetMouseY() << std::endl;
			/*if (Input::IsKeyPressed(SDLK_SPACE))
			{
				std::cout << "Space pressed!" << std::endl;
			}*/

			if (Input::IsKeyPressed(FrostKeycode::SPACE))
			{
				std::cout << "FROSTY SPACE YAYA!" << std::endl;
			}

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