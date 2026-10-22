#pragma once
#include "FrostIncludes.h"
#include "Application.h"


extern Frost::Application* Frost::CreateApplication();

int main()
{
	printf("Frost Engine booted up! Hee-Ho!\n");
	auto app = Frost::CreateApplication();
	app->Run();
	delete app;
}