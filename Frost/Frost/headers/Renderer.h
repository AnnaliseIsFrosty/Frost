#pragma once


namespace Frost
{
	class Renderer
	{
	public:
		virtual ~Renderer() {};

		static Renderer* Create(void* windowHandle);

		virtual void Init() = 0;
		virtual void Render() = 0;
		virtual void OnResize() = 0;

	protected:


	private:



	};


}