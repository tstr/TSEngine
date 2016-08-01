/*
	Program entry point
*/

#include "application.h"
#include <tsengine.h>
#include <tscore/types.h>

#ifdef TS_PLATFORM_WIN32

#include <Windows.h>

int WINAPI WinMain(HINSTANCE module, HINSTANCE prevmodule, LPSTR cmdargs, int cmdShow)
{
	ts::SEngineStartupParams startup;
	startup.app = new ts::Application();
	startup.appInstance = (void*)GetModuleHandle(0);
	startup.commandArgs = std::string(cmdargs);
	startup.showWindow = cmdShow;

	ts::gSystem->init(startup);

	return 0;
};

#else

#error no entry point specified for this platform

#endif