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
		//if (Frost::Input::IsKeyPressed(Frost::FrostKeycode::SPACE))
		//{
		//	std::cout << "HAI!" << std::endl;
		//}
	}

};

Frost::Application* Frost::CreateApplication() {
	return new Sandbox();
}