#pragma once
#include "FrostIncludes.h"
#include "Events/Event.h"

// Platform independant

namespace Frost {

	struct WindowProps
	{
		const char* Title;
		unsigned int Width, Height;

		WindowProps(const char* title = "Frost Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title(title), Width(width), Height(height)
		{
		}
	};


	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		inline virtual const void* GetWindow() = 0;

		// Create function has to be implemented per platform
		static Window* Create(const WindowProps& props = WindowProps());
	};


}