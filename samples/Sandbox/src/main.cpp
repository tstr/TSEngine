/*
	Program entry point
*/

#include "Sandbox.h"
#ifdef WIN32
#include <Windows.h>
#endif

using namespace ts;

int main(int argc, char** argv)
{
#ifdef WIN32
	//Disable dpi virtualization
	SetProcessDPIAware();
#endif

	Sandbox app(argc, argv);
	return app.start();
}
