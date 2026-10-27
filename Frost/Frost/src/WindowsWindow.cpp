#include "WindowsWindow.h"
#include <SDL3/SDL.h>
#include "FrostIncludes.h"

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
		if (s_SDLInitialised) { return; }

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

		impl_->window.reset(SDL_CreateWindow(m_Data.Title, m_Data.Width, m_Data.Height, 0));
		//impl_->window = std::make_unique<SDL_Window>(SDL_CreateWindow(m_Data.Title, m_Data.Width, m_Data.Height, 0));
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
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

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

}