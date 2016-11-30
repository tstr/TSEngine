/*
	Platform console API header
*/

#pragma once

#include <tsengine/abi.h>
#include <tscore/types.h>
#include <iostream>

namespace ts
{
	void TSENGINE_API consoleOpen();
	void TSENGINE_API consoleClose();

	typedef void(*ConsoleClosingHandler_t)();

	void TSENGINE_API setConsoleClosingHandler(ConsoleClosingHandler_t f);

	//void consoleWrite(const char* str, size_t count);
	//void consoleRead(char* buffer, size_t count);
}
