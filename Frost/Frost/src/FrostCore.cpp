#include "FrostCore.h"
#include <SDL3/SDL.h>

namespace Frost {

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

		impl_

	}

}