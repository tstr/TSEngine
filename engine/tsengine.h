/*
	Engine declaration
*/

#pragma once

namespace ts
{
	class Window;

	class ApplicationCore
	{
	private:
		
		Window* m_window = nullptr;
		
	public:
		
		ApplicationCore(const char* cmdline);
		~ApplicationCore();
		
		virtual void onInit() {}
		virtual void onShutdown() {}
		virtual void onUpdate() {}
		virtual void onRender() {}
	};
};