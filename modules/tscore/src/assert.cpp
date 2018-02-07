/*
	assert definition
*/

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#include <windows.h>
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
					<< "line = " << line << "\n"
					<< "lasterr = 0x" << hex << GetLastError() << "\n";

				tserror(s.str());

				if (MessageBoxA(0, s.str().c_str(), "Assert", MB_ICONERROR | MB_OKCANCEL) == IDCANCEL)
				{
					throw exception(s.str().c_str());
				}

				exit(EXIT_FAILURE);
			}
		}
	}
}