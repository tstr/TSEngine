
#include <C3E\core\corecommon.h>
#include <C3E\core\assert.h>
#include <C3E\core\console.h>
#include <C3Ext\win32.h>

#include <sstream>

namespace C3E
{
	namespace Internal
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
					ConsolePrintline(s.str());
					SetConsoleTextAttribute(hConsole, 7);
				}

				MessageBoxA(0, s.str().c_str(), "Assert", MB_ICONERROR);

				exit(EXIT_FAILURE);
			}
		}
	}
}