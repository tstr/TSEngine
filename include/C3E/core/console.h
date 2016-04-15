/*
	Command console header:

	- Only one instance per process allowed
*/

#pragma once

#include <C3E\core\corecommon.h>
#include <iostream>
#include <functional>

//#define NO_CONSOLE

namespace C3E
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

	C3E_CORE_API int ConsoleCreateInstance();
	C3E_CORE_API void ConsoleDestroyInstance();

	C3E_CORE_API void ConsoleSetCommand(const char*, const ConsoleCommandDelegate&);

	C3E_CORE_API void ConsolePrintline(std::string);
	C3E_CORE_API std::string ConsoleReadline();
}

//////////////////////////////////////////////////////////////////////////////