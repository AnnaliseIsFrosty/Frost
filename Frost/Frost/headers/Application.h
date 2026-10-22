#pragma once
#include "FrostIncludes.h"

namespace Frost {
	class Application {
	public: 
		Application();
		virtual ~Application();

		void Run();

	private:

	};

	// To be defined in client
	Application* CreateApplication();
}