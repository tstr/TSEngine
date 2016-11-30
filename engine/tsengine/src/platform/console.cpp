/*
	Platform console API source
*/

#include "console.h"

#include <Windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/memory.h>
#include <tscore/system/thread.h>

static bool g_active = false;

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CConsoleLogStream : public ILogStream
{
private:

	mutex m_mutex;
	stringstream m_stringbuffer;
	
public:
	
	void write(const SLogMessage& msg) override
	{
		if (HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE))
		{
			lock_guard<mutex>lk(m_mutex);

			m_stringbuffer << "[" << msg.function.str() << ":" << msg.line << "] " << msg.message.str();
			
			const WORD defattrib = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

			WORD attrib = defattrib;

			switch (msg.level)
			{
			case eLevelInfo:
				break;
			case eLevelError:
				attrib = FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			case eLevelWarn:
				attrib = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case eLevelProfile:
				attrib = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			}

			SetConsoleTextAttribute(hCon, attrib);	 //set text colour
			printf(m_stringbuffer.str().c_str());
			printf("\n");
			SetConsoleTextAttribute(hCon, defattrib);//reset text colour
			
			m_stringbuffer.str("");
			m_stringbuffer.clear();
		}
	}
};

CConsoleLogStream& getConsoleStreamInstance()
{
	static CConsoleLogStream logstream;
	return logstream;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	void consoleOpen()
	{
		//Check if application does not have a console already open
		if (!g_active)
		{
			AllocConsole();

			if (int code = AttachConsole(GetCurrentProcessId()))
			{
				tserror("AttachConsole failed with code %", code);
			}

			FILE* fp = nullptr;
			errno_t e = 0;

			tsassert(!(e = freopen_s(&fp, "CONOUT$", "w", stdout)));
			tsassert(!(e = freopen_s(&fp, "CONIN$", "r", stdin)));
			//tsassert(!(e = freopen_s(&fp, "CONERR$", "w", stderr))); //todo: fix opening of stderr

			//Synchronise with standard library console functions/objects (cout/cin/cerr)
			std::ios::sync_with_stdio();

			ts::global::getLogger().addStream(&getConsoleStreamInstance());

			g_active = true;
		}
	}

	void consoleClose()
	{
		if (!g_active)
			return;

		//fclose(stdout);
		//fclose(stderr);
		//fclose(stdin);

		ts::global::getLogger().detachStream(&getConsoleStreamInstance());
		
		//WriteConsoleA(GetStdHandle(STD_INPUT_HANDLE), L"\r", 1, NULL, NULL);

		FreeConsole();

		g_active = false;
	}

	static ConsoleClosingHandler_t s_handler;

	static BOOL WINAPI internalConsoleClosingHandler(DWORD ctrltype)
	{
		if (ctrltype == CTRL_CLOSE_EVENT)
			if (s_handler)
				s_handler();

		return true;
	}

	void setConsoleClosingHandler(ConsoleClosingHandler_t f)
	{
		s_handler = f;
		tsassert(SetConsoleCtrlHandler(internalConsoleClosingHandler, true));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////