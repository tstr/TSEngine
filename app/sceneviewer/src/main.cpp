/*
	Program entry point
*/

#include <tsconfig.h>
#include <tsengine.h>
#include "application.h"

#ifdef TS_PLATFORM_WIN32
#include <windows.h>
#endif

int main(int argc, char** argv)
{
	//Set startup parameters
	ts::SEngineStartupParams startup;
	startup.argc = argc;
	startup.argv = argv;

#ifdef TS_PLATFORM_WIN32

	startup.appInstance = (void*)GetModuleHandle(0);
	startup.showWindow = SW_SHOWDEFAULT;

	char path[MAX_PATH];
	GetModuleFileNameA((HMODULE)startup.appInstance, path, MAX_PATH);
	startup.appPath = path;

#endif
	
	//Run engine
	ts::CEngineEnv engine(startup);
	ts::Application app(engine);
	return engine.start(app);
};
