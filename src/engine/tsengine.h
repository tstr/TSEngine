	
#pragma once

namespace tse
{
	class Application
	{
	private:
		
		Window* m_window = nullptr;
		
		
	public:
		
		Application(const char* cmdline);
		~Application();
	};
};