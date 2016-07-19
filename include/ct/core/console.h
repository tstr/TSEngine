/*
	Command console header:

	- Only one instance per process allowed
*/

#pragma once

#include <ct\core\corecommon.h>
#include <iostream>
#include <functional>

//#define NO_CONSOLE

namespace CT
{
	//////////////////////////////////////////////////////////////////////////////

	using std::cout;
	using std::cerr;
	using std::cin;
	using std::endl;

	typedef std::function<void(const char*)> ConsoleCommandDelegate;
		
	/*
	class IConsoleCommand
	{
		virtual const char* GetCommandString() const = 0;
		virtual const char* GetCommandMetaString() const { return nullstr(); }

		virtual void execute(const char* args) = 0;
	};
	*/

	CT_CORE_API int ConsoleCreateInstance();
	CT_CORE_API void ConsoleDestroyInstance();

	CT_CORE_API void ConsoleSetCommand(const char*, const ConsoleCommandDelegate&);

	CT_CORE_API void ConsolePrintline(std::string);
	CT_CORE_API std::string ConsoleReadline();
}

//////////////////////////////////////////////////////////////////////////////