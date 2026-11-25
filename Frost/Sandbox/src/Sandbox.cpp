#include <Frost.h>
#include <../headers/Input.h>

class Sandbox : public Frost::Application
{
public : 
	Sandbox()
	{
	}
	~Sandbox()
	{

	}

	virtual void Run() override
	{
		//std::cout << "App running" << std::endl;
		//std::cout << Frost::Input::GetMouseX() << std::endl;
		//std::cout << timer.GetTime() << std::endl;

	}

private:
};


//Frost::Input* Frost::Input::s_Instance = new Frost::WindowsInput();

Frost::Application* Frost::CreateApplication() {
	return new Sandbox();
}