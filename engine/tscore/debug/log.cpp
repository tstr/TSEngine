/*
	Logging api
*/

#include "log.h"
#include <iostream>
#include <sstream>
#include <Windows.h>

#include <ctime>
#include <iomanip>

using namespace ts;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CLog::operator()(
	const char* message,
	ELogLevel level,
	char const* function,
	char const* file,
	int line)
{
	stringstream ss;
	ss << "(" << function << ":" << line << ") >>> " << message;

	m_stream->write(ss.str().c_str(), level);
}

void CDefaultLogStream::write(const char* message, ELogLevel level)
{

	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hCon)
	{
		const WORD defattrib = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

		WORD attrib = defattrib;

		switch (level)
		{
		case eLevelDebug:
			break;
		case eLevelError:
			attrib = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		case eLevelWarn:
			attrib = FOREGROUND_RED | BACKGROUND_GREEN | FOREGROUND_INTENSITY;
			break;
		case eLevelProfile:
			attrib = FOREGROUND_GREEN;
			break;
		}

		SetConsoleTextAttribute(hCon, attrib);	 //set text colour
		printf(message);
		printf("\n");
		SetConsoleTextAttribute(hCon, defattrib);//reset text colour
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////