/*
	assert definition
*/

#include "assert.h"

#include <Windows.h>
#include <sstream>

namespace ts
{
	namespace internal
	{
		void _internal_assert(
			const char* file,
			const char* func,
			const char* expr,
			int line,
			bool a
			)
		{
			using namespace std;

			if (!a)
			{
				stringstream s;

				s << "Assertion failed:\n"
					<< "function = '" << func << "'\n"
					<< "file = '" << file << "'\n"
					<< "expression = '" << expr << "'\n"
					<< "line = " << line << endl;

				if (HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE))
				{
					SetConsoleTextAttribute(hConsole, 79);
					printf(s.str().c_str());
					SetConsoleTextAttribute(hConsole, 7);
				}

				MessageBoxA(0, s.str().c_str(), "Assert", MB_ICONERROR);

				exit(EXIT_FAILURE);
			}
		}
	}
}