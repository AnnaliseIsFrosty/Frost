#pragma once
#include "KeyCodes.h"


namespace Frost
{
	//enum class MouseButtonFlags
	//{
	//	BUTTON_LEFT = 1,
	//	BUTTON_MIDDLE = 2,
	//	BUTTON_RIGHT = 3,
	//};


	class Input
	{
	public:
		inline static bool IsKeyPressed(FrostKeycode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(FrostMouseButton button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(FrostKeycode keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(FrostMouseButton button) = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Instance;

	};

}