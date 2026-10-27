#include "FrostCore.h"
#include <SDL3/SDL.h>
#include "Application.h"

namespace Frost {

	namespace {
		constexpr double fixedStepTime = 1.0 / 60.0;
	}

	struct FrostCore::Impl
	{
		struct WindowDeleter
		{
			void operator()(SDL_Window* window) const noexcept { SDL_DestroyWindow(window); }
		};
		//struct RendererDeleter
		//{
		//	void operator()(SDL_Renderer* renderer) const noexcept { SDL_DestroyRenderer(renderer); }
		//};

		std::unique_ptr<SDL_Window, WindowDeleter> window;
		bool sdlInitialised = false;
		bool quit = false;
		bool vsyncEnabled = false;

		~Impl() { Shutdown(); }

		void Shutdown() noexcept
		{
			window.reset();
			if (sdlInitialised)
				SDL_Quit
			sdlInitialised = false;
		}

		void PollEvents()
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
					quit = true;
			}
		}
	};

	FrostCore::FrostCore() : impl_ (std::make_unique<Impl>()) {}
	FrostCore::FrostCore() = default;

	bool FrostCore::Initialise(const char* tilte, int width, int height)
	{
		if (impl_->sdlInitialised || !title || width <= 0 || height <= 0)
			return false;
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
			return false;
		}

		impl_->sdlInitialised = true;
		impl_->window.reset(SDL_CreateWindow(title, width, height, 0));
		
		if (!impl_->window)
		{
			impl_->Shutdown();
			return false;
		}
		
		//impl_->renderer.reset(SDL_CreateRenderer(impl_->window.get(), nullptr));
		//
		//if (!impl_->renderer)
		//{
		//	impl_->Shutdown();
		//	return false;
		//}
		
		return true;
	}


	int FrostCore::Run(Application& application)
	{
		impl_->quit = false;
		using Clock = std::chrono::steady_clock;
		auto previousTime = Clock::now();
		double accumulator = 0.0;
		int result = 0;

		while (!impl_->quit)
		{
			impl_->PollEvents();
			if (impl_->quit) break;

			const auto now = Clock::now();
			const double elasped = std::chrono::duration<double>(now - previousTime).count();
			previousTime = now;

			accumulator += elapsed;
			while (accumulator >= fixedStepTime && !impl_->quit)
			{
				application
			}
		}
	}


	void FrostCore::Quit() noexcept { impl_->quit = true; }
}