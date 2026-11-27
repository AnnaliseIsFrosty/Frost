#pragma once
#include "FrostIncludes.h"

namespace Frost
{

	class Timer
	{
	public:
		Timer();
		~Timer() {};

		void CalculateDeltaTime();
		double GetTime(); // Returns time elapsed since start of application in seconds
		double GetDeltaTime() { return dt; };

	private:
		uint64_t NOW;
		uint64_t LAST;
		double dt;

	};

}