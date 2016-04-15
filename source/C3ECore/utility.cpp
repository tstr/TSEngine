/*
	Utility source
*/

#include "pch.h"

#include <C3Ext\win32.h>
#include <C3E\Core\utility.h>
#include <C3E\Core\console.h>

#include <sstream>

#include <Psapi.h>
#include <rpc.h>

LINK_LIB("rpcrt4.lib")

#ifdef _DEBUG

#include <DbgHelp.h>
LINK_LIB("Dbghelp.lib")

#endif

using namespace std;

namespace C3E
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t GetProcessorCount()
	{
		SYSTEM_INFO info; GetSystemInfo(&info);

		return (UINT)info.dwNumberOfProcessors;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	string GetExcecutableDirectory()
	{
		char buffer[MAX_PATH];
		GetModuleFileNameA(GetModuleHandleA(NULL), buffer, MAX_PATH);

		string temp(buffer);
		size_t pos = temp.find_last_of('\\');

		if (pos != string::npos) temp.erase(pos + 1, temp.length());

		return temp.c_str();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	string GetProcessorName()
	{
		int CPUInfo[4] = { -1 };

		unsigned nExIds, i = 0;

		char CPUname[0x40];

		__cpuid(CPUInfo, 0x80000000);

		nExIds = CPUInfo[0];
		for (i = 0x80000000; i <= nExIds; ++i)
		{
			__cpuid(CPUInfo, i);

			if (i == 0x80000002)
				memcpy(CPUname, CPUInfo, sizeof(CPUInfo));

			else if (i == 0x80000003)
				memcpy(CPUname + 16, CPUInfo, sizeof(CPUInfo));

			else if (i == 0x80000004)
				memcpy(CPUname + 32, CPUInfo, sizeof(CPUInfo));

		}

		std::string buffer(CPUname);

		while (buffer.front() == ' ') buffer.erase(0, 1);

		return buffer.c_str();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t GetMaxMemoryCapacity()
	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);

		GlobalMemoryStatusEx(&status);
		return ((size_t)status.ullTotalPhys / 1024);
	}

	size_t GetMemoryUsage()
	{
		PROCESS_MEMORY_COUNTERS pmc;

		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS));

		return (pmc.WorkingSetSize / 1024);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CopyToClipboard(const char* str)
	{
		const size_t len = strlen(str) + 1;

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), str, len);
		GlobalUnlock(hMem);

		OpenClipboard(0);

		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);

		CloseClipboard();
	}

	string CopyFromClipboard()
	{
		if (!OpenClipboard(0))
			return 0;

		HGLOBAL h = GetClipboardData(CF_TEXT);

		if (h == NULL)
			return 0;

		char* str = static_cast<char*>(GlobalLock(h));

		if (!str)
			return 0;

		string buffer(str);

		GlobalUnlock(h);

		CloseClipboard();

		return buffer.c_str();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t GetMonitorSizeX() { return GetSystemMetrics(SM_CXSCREEN); }
	size_t GetMonitorSizeY() { return GetSystemMetrics(SM_CYSCREEN); }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MsgBox(const char* text, const char* header)
	{
		return MessageBoxA(0, text, 0, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	string PrintCallstack()
	{
		typedef USHORT(WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
		CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace"));

		if (func == NULL)
			return "";

		const int kMaxCallers = 62;

		stringstream stream;

		void* callers[kMaxCallers];
		int count = (func)(0, kMaxCallers, callers, NULL);

		for (int i = 0; i < count; i++)
		{
			stream << i << " called from " << callers[i] << endl;
		}

		return stream.str();
	}

	void GenerateMinidump(const char* fn)
	{

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint64_t GenerateUID()
	{
		cout << sizeof(UUID);

		UUID uid;
		UuidCreate(&uid);

		return 0;
		//return (reinterpret_cast<uint64_t>(uid));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}