/*
	System information source
*/

#include "info.h"

#include <Windows.h>
#include <Psapi.h>

static std::string getCPUname();

namespace ts
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void getSystemInformation(SSystemInfo& info)
	{
		ZeroMemory(&info, sizeof(SSystemInfo));

		//Get computer username
		char username[64];
		DWORD usernameLength = 64;
		GetUserName(username, &usernameLength);
		info.userName = username;

		//OS info

		//CPU info
		SYSTEM_INFO sysinf;
		ZeroMemory(&sysinf, sizeof(SYSTEM_INFO));
		GetSystemInfo(&sysinf);
		
		info.cpuProcessorCount = sysinf.dwNumberOfProcessors;
		
		switch (sysinf.wProcessorArchitecture)
		{
		case (PROCESSOR_ARCHITECTURE_AMD64) :
			info.cpuArchitecture = eCPUarchX64; break;
		case (PROCESSOR_ARCHITECTURE_ARM) :
			info.cpuArchitecture = eCPUarchARM; break;
		case (PROCESSOR_ARCHITECTURE_INTEL) :
			info.cpuArchitecture = eCPUarchX86; break;
		}
		
		info.cpuName = getCPUname();

		info.numDisplays = GetSystemMetrics(SM_CMONITORS);

	}

	void getPrimaryDisplayInformation(SDisplayInfo& info)
	{
		info.width = GetSystemMetrics(SM_CXSCREEN);
		info.height = GetSystemMetrics(SM_CYSCREEN);

		info.colourDepth = GetDeviceCaps(GetDC(0), BITSPIXEL);
	}

	void getSystemMemoryInformation(SSystemMemoryInfo& info)
	{
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);

		info.mCapacity = status.ullTotalPhys / 1024;
		//info.memGlobalPercentageUsage = (uint8)status.dwMemoryLoad;

		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
		
		info.mUsed = (uint64)pmc.WorkingSetSize / 1024;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

std::string getCPUname()
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////