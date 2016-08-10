/*
	Program entry point
*/

#include "application.h"
#include <tsengine.h>
#include <tscore/types.h>
#include <tscore/system/error.h>

#define APP_CLASS ts::Application

#ifdef TS_PLATFORM_WIN32

#include <windows.h>

int WINAPI WinMain(HINSTANCE module, HINSTANCE prevmodule, LPSTR cmdargs, int cmdShow)
{
	//Initialize debug layer
	ts::internal::initializeSystemExceptionHandlingFilter();	//This sets the global exception handling filter for the process, meaning uncaught exceptions can be detected and a minidump can be generated
#ifdef _DEBUG
	ts::internal::initializeSystemMemoryLeakDetector();			//This allows the crt to detect memory leaks
#endif

	//Set startup parameters
	ts::SEngineStartupParams startup;
	startup.app = new APP_CLASS;
	startup.appInstance = (void*)GetModuleHandle(0);
	startup.commandArgs = std::string(cmdargs);
	startup.showWindow = cmdShow;

	char path[MAX_PATH];
	GetModuleFileNameA((HMODULE)startup.appInstance, path, MAX_PATH);
	startup.appPath = path;

	//Run engine
	ts::CEngineSystem engine(startup);

	return 0;
};

#else

#error no entry point specified for this platform

#endif