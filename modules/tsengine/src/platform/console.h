/*
	Platform console API header
*/

#pragma once

#include <tsengine/abi.h>
#include <tscore/types.h>
#include <iostream>

namespace ts
{
	void consoleOpen();
	void consoleClose();

	typedef void(*ConsoleClosingHandler_t)();

	void setConsoleClosingHandler(ConsoleClosingHandler_t f);

	//void consoleWrite(const char* str, size_t count);
	//void consoleRead(char* buffer, size_t count);
}
