#include "WindowsInput.h"
#include <SDL3/SDL.h>
#include "FrostIncludes.h"

namespace Frost
{
	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		const bool* keyboardState = SDL_GetKeyboardState(NULL);
		if (keyboardState[SDL_GetScancodeFromKey(keycode, NULL)])
		{
			std::cout << "Key pressed!" << std::endl;
			return true;
		}
		else {
			return false;
		}
	}

	bool WindowsInput::IsMouseButtonPressedImpl(MouseButtonFlags button)
	{
		//float x;
		//float y;
		SDL_MouseButtonFlags mouseState = SDL_GetMouseState(NULL, NULL);
		return mouseState && SDL_BUTTON_MASK((int)button);
	}

	float WindowsInput::GetMouseXImpl()
	{
		float x;
		SDL_MouseButtonFlags mouseState = SDL_GetMouseState(&x, NULL);
		return x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		float y;
		SDL_MouseButtonFlags mouseState = SDL_GetMouseState(NULL, &y);
		return y;
	}

}