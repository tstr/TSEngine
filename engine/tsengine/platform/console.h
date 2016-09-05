/*
	Platform console API header
*/

#pragma once

#include <tscore/types.h>
#include <iostream>

namespace ts
{
	void TSCORE_API consoleOpen();
	void TSCORE_API consoleClose();

	typedef void(*ConsoleClosingHandler_t)();

	void TSCORE_API setConsoleClosingHandler(ConsoleClosingHandler_t f);

	//void consoleWrite(const char* str, size_t count);
	//void consoleRead(char* buffer, size_t count);
}
