#pragma once


namespace Frost
{
	class Renderer
	{
	public:
		virtual ~Renderer() {};

		static Renderer* Create(void* windowHandle);

		virtual void Init() = 0;
		virtual void Render(double time) = 0;
		virtual void OnResize(float width, float height) = 0;

	protected:


	private:



	};


}