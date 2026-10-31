#include "WindowsWindow.h"
#include <SDL3/SDL.h>
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Frost {

	static bool s_SDLInitialised = false;

	struct WindowsWindow::SDL_Impl
	{
		struct WindowDeleter
		{
			void operator()(SDL_Window* window) const noexcept { if (window) { SDL_DestroyWindow(window); } }
		};
		
		std::unique_ptr<SDL_Window, WindowDeleter> window;

		~SDL_Impl() { Shutdown(); }

		void Shutdown() noexcept
		{
			window.reset();
			if (s_SDLInitialised)
				SDL_Quit();

			s_SDLInitialised = false;
		}
	};

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		impl_ = std::make_unique<SDL_Impl>();
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		std::cout << "Initialising window for Windows!" << std::endl;

		if (!s_SDLInitialised) {
			SDL_Init(SDL_INIT_VIDEO);
			s_SDLInitialised = true;
		}


		// DO SDL WINDOW CREATION HERE
		// ---------------------------

		impl_->window.reset(SDL_CreateWindow(m_Data.Title, m_Data.Width, m_Data.Height, SDL_WINDOW_RESIZABLE));
		if (!impl_->window)
		{
			Shutdown();
			return;
		}

		SetVSync(true);
	}

	void WindowsWindow::Shutdown()
	{
		// DO SDL WINDOW DELETION HERE
		// ---------------------------
		impl_->Shutdown();
	}

	void WindowsWindow::OnUpdate()
	{
		// DO SDL EVENT POLLING HERE
		// DO SDL BUFFER SWAPPING HERE
		SDL_Event sEvent;
		while (SDL_PollEvent(&sEvent))
		{
			switch (sEvent.type)
			{
			
			case SDL_EVENT_WINDOW_RESIZED:
			{
				std::cout << "WINDOW RESIZED : " << sEvent.window.data1 << " ; " << sEvent.window.data2 << std::endl;
				WindowResizeEvent event(sEvent.window.data1, sEvent.window.data2);
				m_Data.EventCallback(event);
				break;
			}
			
			case SDL_EVENT_QUIT:
			{
				std::cout << "QUITTING WINDOW" << std::endl;
				WindowCloseEvent event = WindowCloseEvent();
				m_Data.EventCallback(event);
				break;
			}
			
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				std::cout << "QUITTING WINDOW" << std::endl;
				WindowCloseEvent event = WindowCloseEvent();
				m_Data.EventCallback(event);
				break;
			}
			
			case SDL_EVENT_KEY_DOWN:
			{
				std::cout << "KEY PRESSED : " << sEvent.key.key << std::endl;
				KeyPressedEvent event(sEvent.key.key, sEvent.key.repeat);
				m_Data.EventCallback(event);
				break;
			}
			
			case SDL_EVENT_KEY_UP:
			{
				std::cout << "KEY RELEASED : " << sEvent.key.key << std::endl;
				KeyReleasedEvent event(sEvent.key.key);
				m_Data.EventCallback(event);
				break;
			}

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				std::cout << "MOUSE BUTTON DOWN : " << (int)sEvent.button.button << std::endl;
				MouseButtonPressedEvent event((int)sEvent.button.button);
				m_Data.EventCallback(event);
				break;
			}
			
			case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				std::cout << "MOUSE BUTTON UP : " << (int)sEvent.button.button << std::endl;
				MouseButtonReleasedEvent event((int)sEvent.button.button);
				m_Data.EventCallback(event);
				break;
			}

			case SDL_EVENT_MOUSE_WHEEL:
			{
				std::cout << "MOUSE SCROLLED : " /*<< sEvent.button.button */<< std::endl;
				MouseScrolledEvent event(sEvent.wheel.x, sEvent.wheel.y);
				m_Data.EventCallback(event);
				break;
			}
			
			case SDL_EVENT_MOUSE_MOTION:
			{
				//std::cout << "MOUSE MOVED :" << std::endl;
				MouseMovedEvent event(sEvent.motion.x, sEvent.motion.y);
				m_Data.EventCallback(event);
				break;
			}

			default:
				//std::cout << "Unhandled event" << std::endl;
				break;
			}
		}
	}

	void WindowsWindow::SetVSync(bool isEnabled)
	{
		if (isEnabled)
		{
			// DO SDL VSYNC ENABLING HERE
		}

		m_Data.VSync = isEnabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	inline const void* WindowsWindow::GetWindow()
	{
		return impl_->window.get();
	}
}