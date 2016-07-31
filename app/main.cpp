/*
	Program entry point
*/

#include "application.h"
#include <tsconfig.h>
#include <tscore/platform/console.h>

#ifdef TS_PLATFORM_WIN32

#include <Windows.h>

int WINAPI WinMain(HINSTANCE module, HINSTANCE prevmodule, LPSTR cmdargs, int cmdShow)
{
	struct Console
	{
		Console() { ts::consoleOpen(); }
		~Console() { ts::consoleClose(); }
	} cnsl;

	ts::Application app(cmdargs);
	return 0;
};

#else

#error no entry point specified for this platform

#endif