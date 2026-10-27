//#pragma once
//#include "FrostIncludes.h"
//
//namespace Frost {
//
//	class FrostCore;
//
//	extern class Application;
//
//	class FrostCore final
//	{
//	public:
//		FrostCore();
//		~FrostCore();
//
//		FrostCore(const FrostCore&) = delete;
//		FrostCore& operator=(const FrostCore&) = delete;
//		FrostCore(FrostCore&&) = delete;
//		FrostCore& operator=(FrostCore&&) = delete;
//
//		bool Initialise(const char* title, int width, int height);
//		int Run(Application& application);
//		void Quit() noexcept;
//
//	private:
//		struct Impl;
//		std::unique_ptr<Impl> impl_;
//	};
//
//
//}