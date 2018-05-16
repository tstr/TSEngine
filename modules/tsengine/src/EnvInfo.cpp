/*
	System information source
*/

#include <tsengine/App.h>

#include <windows.h>
#include <Psapi.h>

#include <tscore/debug/log.h>

using namespace std;
using namespace ts;

static void getCPUname(string& name, const string& default = "");
static void getOSname(string& name, const string& default = "");
static ECpuVendorID getCPUvendorID();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Path Application::getCurrentDir() const
{
	char path[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path);

	return Path(path);
}

Path Application::getBinaryDir() const
{
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);

	return Path(path).getParent();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Application::getSystemInfo(SSystemInfo& info)
{
	//Get computer username
	char username[64];
	DWORD usernameLength = 64;
	GetUserName(username, &usernameLength);
	info.userName = username;
	
	//OS info
	getOSname(info.osName, "Unknown");

	//CPU info
	SYSTEM_INFO sysinf;
	ZeroMemory(&sysinf, sizeof(SYSTEM_INFO));
	GetSystemInfo(&sysinf);
	
	info.cpuProcessorCount = sysinf.dwNumberOfProcessors;
	
	switch (sysinf.wProcessorArchitecture)
	{
	case (PROCESSOR_ARCHITECTURE_AMD64) :
		info.cpuArchitecture = eCPUarchAMD64; break;
	case (PROCESSOR_ARCHITECTURE_ARM) :
		info.cpuArchitecture = eCPUarchARM; break;
	case (PROCESSOR_ARCHITECTURE_INTEL) :
		info.cpuArchitecture = eCPUarchX86; break;
	}
	
	getCPUname(info.cpuName, "Unknown");
	info.cpuVendorID = getCPUvendorID();

	info.numDisplays = GetSystemMetrics(SM_CMONITORS);

}

void Application::getSystemDisplayInfo(SDisplayInfo& info)
{
	info.width = GetSystemMetrics(SM_CXSCREEN);
	info.height = GetSystemMetrics(SM_CYSCREEN);

	info.colourDepth = GetDeviceCaps(GetDC(0), BITSPIXEL);
}

void Application::getSystemMemoryInfo(SSystemMemoryInfo& info)
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);

	info.mCapacity = status.ullTotalPhys;
	//info.memGlobalPercentageUsage = (uint8)status.dwMemoryLoad;

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
	
	info.mUsed = (uint64)pmc.WorkingSetSize / 1024;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void getOSname(string& name, const string& default)
{
	HKEY hKey = 0;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
	
	name = default;

	if (result == ERROR_SUCCESS)
	{
		BYTE buffer[128];
		DWORD buffersize = sizeof(buffer);
		result = RegQueryValueEx(hKey, "ProductName", 0, 0, buffer, &buffersize);

		if (result == ERROR_SUCCESS)
		{
			name = (const char*)buffer;
		}
	}
	else if (result == ERROR_FILE_NOT_FOUND)
	{
		tserror("ERROR_FILE_NOT_FOUND");
	}
}

ECpuVendorID getCPUvendorID()
{
	int regs[4] = { 0 };
	char vendor[13];
	__cpuid(regs, 0);              // mov eax,0; cpuid
	memcpy(vendor, &regs[1], 4);   // copy EBX
	memcpy(vendor + 4, &regs[3], 4); // copy EDX
	memcpy(vendor + 8, &regs[2], 4); // copy ECX
	vendor[12] = '\0';

	if (compare_string_weak(vendor, "GenuineIntel"))
	{
		return eCpuIntel;
	}
	if (compare_string_weak(vendor, "AuthenticAMD"))
	{
		return eCpuAMD;
	}

	return eCpuUnknown;
}

void getCPUname(string& buffer, const string& default)
{
	int CPUInfo[4] = { -1 };

	unsigned nExIds, i = 0;

	char CPUname[0x40];
	ZeroMemory(CPUname, sizeof(CPUname));

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

	string CPUnameBuffer(CPUname);

	while (CPUnameBuffer.front() == ' ')
		CPUnameBuffer.erase(0, 1);

	buffer = move(CPUnameBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
