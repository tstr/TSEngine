/*
	Logging api
*/

#include "log.h"
#include <iostream>

using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CLog::operator()(
	const char* message,
	ELogLevel level,
	char const* function,
	char const* file,
	int line)
{
	m_stream->write(message, level);
}

void CDefaultLogStream::write(const char* message, ELogLevel level)
{
	std::cout << message << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////