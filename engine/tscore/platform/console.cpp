/*
	Platform console API source
*/

#include "console.h"

#include <Windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>

#include <tscore/debug/assert.h>

static bool g_active = false;

namespace ts
{
	void consoleOpen()
	{
		//Check if application does not have a console
		if (GetConsoleCP() == 0 && !g_active)
		{
			tsassert(AllocConsole());

			FILE* fp = nullptr;
			errno_t e = 0;
			
			tsassert(!(e = freopen_s(&fp, "CONOUT$", "w", stdout)));
			tsassert(!(e = freopen_s(&fp, "CONIN$", "r", stdin)));
			tsassert(!(e = freopen_s(&fp, "CONERR$", "w", stderr)));

			//Synchronise with standard library console functions/objects
			std::ios::sync_with_stdio();

			g_active = true;
		}
	}

	void consoleClose()
	{
		if (!g_active)
			return;

		fclose(stdout);
		fclose(stdin);
		fclose(stderr);

		g_active = false;
	}
}
