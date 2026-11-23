#include "Timer.h"
#include <SDL3/SDL.h>

namespace Frost
{

	double Timer::GetTime()
	{
		return static_cast<double>(SDL_GetTicksNS()) * 0.000000001;
	}

}