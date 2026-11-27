#include "Timer.h"
#include <SDL3/SDL.h>

namespace Frost
{
	Timer::Timer()
	{
		NOW = SDL_GetPerformanceCounter();
		LAST = 0;
		dt = 0.f;
	}

	void Timer::CalculateDeltaTime()
	{
		// Code learned from https://gamedev.stackexchange.com/questions/110825/how-to-calculate-delta-time-with-sdl

		LAST = NOW;
		NOW = SDL_GetPerformanceCounter();

		dt = (double)(((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency()) * 0.001);
	}

	double Timer::GetTime()
	{
		return static_cast<double>(SDL_GetTicksNS()) * 0.000000001;
	}

}