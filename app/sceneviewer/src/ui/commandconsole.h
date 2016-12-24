/*
	Command console header
*/

#pragma once

#include "ui.h"
#include <tscore/debug/log.h>
#include <tscore/strings.h>
#include <tscore/system/thread.h>

#include <vector>
#include <functional>
#include <map>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct SCommandConsoleCallbackArgs
	{
		std::string params;
	};

	typedef std::function<void(const SCommandConsoleCallbackArgs&)> CommandConsoleCallback;

	class UICommandConsole : public ILogStream
	{
	private:

		struct SLine
		{
			ImColor colour;
			std::string text;

			SLogMessage logmeta;
		};

		std::map<std::string, CommandConsoleCallback> m_commandTable;

		StaticString<4000> m_readBuffer;
		std::recursive_mutex m_lineBufferMutex;
		std::vector<SLine> m_lineBuffer;
		bool m_lineScroll = false;
		
		Application* m_app;
	
		void write(const SLogMessage& msg) override;
		int textCallback(ImGuiTextEditCallbackData* data);

	public:
		
		UICommandConsole(Application* app);
		~UICommandConsole();

		void setCommand(const char* command, const CommandConsoleCallback& callback);
		void executeCommand(const char* command);
		
		void show();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
