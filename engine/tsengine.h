/*
	Engine declaration
*/

#pragma once

namespace ts
{
	class Window;

	class CApplicationCore
	{
	private:
		
		Window* m_window = nullptr;

	public:
		
		CApplicationCore(const char* cmdline);
		~CApplicationCore();
		
		virtual void onInit() {}
		virtual void onShutdown() {}
		virtual void onUpdate() {}
		virtual void onRender() {}
	};
};