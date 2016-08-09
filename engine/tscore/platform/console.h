/*
	Platform console API header
*/

#pragma once

#include <tscore/types.h>
#include <iostream>

namespace ts
{
	void consoleOpen();
	void consoleClose();


	typedef void(*consoleClosingHandler_t)();

	void setConsoleClosingHandler(consoleClosingHandler_t f);

	//void consoleWrite(const char* str, size_t count);
	//void consoleRead(char* buffer, size_t count);
}
