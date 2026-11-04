#pragma once
#include "Input.h"
#include "KeyCodes.h"

namespace Frost
{
	class WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(FrostKeycode keycode) override;

		virtual bool IsMouseButtonPressedImpl(FrostMouseButton button) override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}