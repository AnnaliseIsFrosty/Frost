#include "Application.h"
#include <SDL3/SDL.h>


namespace Frost {

	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback([this](Event& e) {OnEvent(e); });
		
		//HWND wind = (HWND)m_Window->GetWindow();
		m_Renderer = std::unique_ptr<Renderer>(Renderer::Create(m_Window->GetWindow(), &m_Timer));

	}

	Application::~Application() {

	}

	void Application::Run() {
		while (m_Running)
		{
			m_Timer.CalculateDeltaTime();
			//if (Input::IsKeyPressed(FrostKeycode::SPACE))
			//{
			//	std::cout << "FROSTY SPACE YAYA!" << std::endl;
			//}

			m_Window->OnUpdate();
			m_Renderer->Render(m_Timer.GetTime());
		
		
			m_App->Run();

		}
	
	}

	void Application::OnEvent(Event& e)
	{
		//std::cout << "Frosty Event Called! Hee-Ho!" << std::endl;
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& ev) { return OnWindowClose(ev); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& ev) { return OnWindowResize(ev); });
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		std::cout << "RESIZED WINDOW HEE HO!" << std::endl;
		m_Renderer->OnResize(e.GetWidth(), e.GetHeight());
		return true;
	}
}