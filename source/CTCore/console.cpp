/*
	Command console source
*/

#include "pch.h"

#include <CTxt\win32.h>

#include <CT\Core\console.h>
#include <CT\Core\utility.h>
#include <CT\Core\threading.h>
#include <CT\Core\memory.h>

#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include <unordered_map>
#include <string>

using namespace CT;
using namespace std;

struct ConsoleInstance
{
	unordered_map<string, ConsoleCommandDelegate> m_commandTable;
	string m_buffer;
	mutex m_buffer_mutex;

	atomic<bool> m_active = true;
	thread m_subthread;

	ConsoleInstance()
	{
		int hConHandle;
		INT64 lStdHandle;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		FILE *fp;

		//Attach a console to process
		if (!AttachConsole(ATTACH_PARENT_PROCESS))
		{
			if (!AllocConsole())
				throw exception("Console window: Console::Console()");
		}

		//Set console size
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
		coninfo.dwSize.Y = 500;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

		////////////////////////////////////////////////////////////////////////////

		//STDOUT
		lStdHandle = (INT64)GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");

		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);

		//STDIN
		lStdHandle = (INT64)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "r");

		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		//STDERR
		lStdHandle = (INT64)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");

		*stderr = *fp;
		setvbuf(stderr, NULL, _IONBF, 0);

		//Synchronise with standard library console functions/objects
		ios::sync_with_stdio();

		m_active = true;

		m_subthread.swap(thread(&ConsoleInstance::procedure, this));
		//subthread.detach();
	}

	~ConsoleInstance()
	{
		if (!m_active) return;

		m_active = false;

		//cin.rdbuf()->in_avail();

		/*
		fclose(stdout);
		fclose(stdin);
		fclose(stderr);

		CloseHandle(GetStdHandle(STD_OUTPUT_HANDLE));
		CloseHandle(GetStdHandle(STD_INPUT_HANDLE));
		*/

		//Unblock std::cin by giving it something to read
		WriteConsoleA(GetStdHandle(STD_INPUT_HANDLE), L"\r", 1, NULL, NULL);
		FreeConsole();

		m_subthread.join();
	}

	void procedure()
	{
		size_t pos = 0;

		while (m_active)
		{
			//cout << "<Enter command> ";

			getline(cin, m_buffer);

			if (!m_buffer.empty())
			{
				//parse the buffer
				string cmd(m_buffer);

				size_t pos = cmd.find_first_of(' ');

				cmd = cmd.substr(0, pos);

				for (char& c : cmd)
					tolower(c);

				auto it = m_commandTable.find(cmd);

				if (it == m_commandTable.end())
				{
					cout << "Unknown command '" << cmd << "'" << endl;// << endl;
				}
				else
				{
					const ConsoleCommandDelegate& f = it->second;

					if (f == nullptr)
					{
						cout << "Unknown command '" << cmd << "'" << endl;// << endl;
					}
					else
					{
						//extract arguments from buffer
						string args(m_buffer);

						if (pos == string::npos)
							args = "";
						else
							args = m_buffer.substr(pos + 1, m_buffer.size() - 1);

						f(args.c_str());
					}
				}
			}
		}
	}

	void PrintCommandList()
	{
		cout << "\nList of available commands:\n";

		for (auto& c : m_commandTable)
		{
			cout << c.first << endl;
		}

		cout << endl;
	}
};

static std::unique_ptr<ConsoleInstance> g_console;

namespace CT
{
	int ConsoleCreateInstance()
	{
		if (g_console.get())
			return 0;

		g_console.reset(new ConsoleInstance);

		//Preset commands
		ConsoleSetCommand("system", [](const char* cmd){ system(cmd); });
		ConsoleSetCommand("hello", [](const char* cmd){ printf("world"); });
		ConsoleSetCommand("list", [](const char*){ g_console->PrintCommandList(); });

		return S_OK;
	}

	void ConsoleDestroyInstance()
	{
		g_console.reset(nullptr);
	}

	void ConsolePrintline(string str)
	{
		if (g_console.get())
		{
			lock_guard<mutex>lk(g_console->m_buffer_mutex);
			puts(str.c_str());
		}
	}

	string ConsoleReadLine()
	{
		if (g_console.get())
			return g_console->m_buffer;
		else
			return string();
	}

	void ConsoleSetCommand(const char* keyword, const ConsoleCommandDelegate& fnc)
	{
		string str(keyword);
		for (char& c : str)
			tolower(c);

		g_console->m_commandTable[keyword] = fnc;
	}
}