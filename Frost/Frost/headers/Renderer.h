#pragma once


namespace Frost
{
	class Renderer
	{
	public:
		virtual void Init() = 0;
		virtual void Render() = 0;

		virtual ~Renderer() {};

		static Renderer* Create(void* windowHandle);


	protected:


	private:



	};


}