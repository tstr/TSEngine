/*
	global exception filter
*/

#include <C3Ext\win32.h>
#include <C3E\core\console.h>

#include <DbgHelp.h>
#include <debugapi.h>
#include <sstream>
#include <io.h>

#define LOG_EXCEPTION(exc) case exc: { out << #exc; break; } 

using namespace std;

__declspec(noinline)
LONG WINAPI exception_filter(EXCEPTION_POINTERS *info)
{
	stringstream out;
	DWORD code = info->ExceptionRecord->ExceptionCode;
	out << hex << "Unhandled Exception: ";

	switch (code)
	{
		LOG_EXCEPTION(EXCEPTION_ACCESS_VIOLATION)
			LOG_EXCEPTION(EXCEPTION_DATATYPE_MISALIGNMENT)
			LOG_EXCEPTION(EXCEPTION_BREAKPOINT)
			LOG_EXCEPTION(EXCEPTION_SINGLE_STEP)
			LOG_EXCEPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
			LOG_EXCEPTION(EXCEPTION_FLT_DENORMAL_OPERAND)
			LOG_EXCEPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO)
			LOG_EXCEPTION(EXCEPTION_FLT_INEXACT_RESULT)
			LOG_EXCEPTION(EXCEPTION_FLT_INVALID_OPERATION)
			LOG_EXCEPTION(EXCEPTION_FLT_OVERFLOW)
			LOG_EXCEPTION(EXCEPTION_FLT_STACK_CHECK)
			LOG_EXCEPTION(EXCEPTION_FLT_UNDERFLOW)
			LOG_EXCEPTION(EXCEPTION_INT_DIVIDE_BY_ZERO)
			LOG_EXCEPTION(EXCEPTION_INT_OVERFLOW)
			LOG_EXCEPTION(EXCEPTION_PRIV_INSTRUCTION)
			LOG_EXCEPTION(EXCEPTION_IN_PAGE_ERROR)
			LOG_EXCEPTION(EXCEPTION_ILLEGAL_INSTRUCTION)
			LOG_EXCEPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION)
			LOG_EXCEPTION(EXCEPTION_STACK_OVERFLOW)
			LOG_EXCEPTION(EXCEPTION_INVALID_DISPOSITION)
			LOG_EXCEPTION(EXCEPTION_GUARD_PAGE)
			LOG_EXCEPTION(EXCEPTION_INVALID_HANDLE)
	}

	out << "\n(0x" << code << ") at (0x" << (info->ExceptionRecord->ExceptionAddress) << ")";

	C3E::ConsolePrintline(out.str());

	//cerr << out.str() << endl;
	//Core::Utility::CopyToClipboard(out.str().c_str());

	out << "\n\nWould you like to generate debug information?";

	if (MessageBoxA(0, out.str().c_str(), "Crash handler", MB_ICONERROR | MB_YESNO) == IDYES)
	{
		if (HMODULE lib = LoadLibraryA("dbghelp.dll"))
		{
			BOOL(__stdcall *f_writeMinidump)(
				_In_ HANDLE hProcess,
				_In_ DWORD ProcessId,
				_In_ HANDLE hFile,
				_In_ MINIDUMP_TYPE DumpType,
				_In_opt_ PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
				_In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
				_In_opt_ PMINIDUMP_CALLBACK_INFORMATION CallbackParam
				) = nullptr;

			f_writeMinidump = (decltype(f_writeMinidump))GetProcAddress(lib, "MiniDumpWriteDump");

			if (f_writeMinidump)
			{
				FILE* file = nullptr;
				fopen_s(&file, "D3D_minidump.dmp", "w");

				MINIDUMP_EXCEPTION_INFORMATION dump_info;
				dump_info.ExceptionPointers = info;
				dump_info.ClientPointers = true;
				dump_info.ThreadId = GetCurrentThreadId();

				HANDLE hfile = (HANDLE)_get_osfhandle(_fileno(file));

				f_writeMinidump(
					GetCurrentProcess(),
					GetCurrentProcessId(),
					hfile,
					(MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs),
					&dump_info,
					NULL,
					NULL
					);

				char full_path[MAX_PATH];
				ZeroMemory(full_path, MAX_PATH);
				GetFinalPathNameByHandleA(
					hfile,
					full_path,
					MAX_PATH,
					VOLUME_NAME_DOS
					);

				stringstream out;
				out << "Dump file created successfully at: '" << full_path << "'\n";
				cerr << out.str();
				MessageBoxA(0, out.str().c_str(), "Crash handler", 0);

				fclose(file);
			}
			else
			{
				MessageBoxA(0, "Minidump", "Unable to load function 'MiniDumpWriteDump'", MB_ICONWARNING);
			}

			FreeLibrary(lib);
		}
		else
		{
			MessageBoxA(0, "Minidump", "Unable to load library 'debughelp.dll'", MB_ICONWARNING);
		}
	}

	return EXCEPTION_EXECUTE_HANDLER;
}


static const class ExceptionFilter
{
public:

	ExceptionFilter()
	{
		SetUnhandledExceptionFilter(exception_filter);
	}

} g_ef;