/*
	Application definition
*/

#include <iostream>
#include "application.h"

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/strings.h>

using namespace ts;

static void printsysteminfo();

void Application::onInit()
{
	printsysteminfo();
}


static void printsysteminfo()
{
	//CPU information
	SSystemInfo inf;
	getSystemInformation(inf);

	tsinfo("CPU name: %", inf.cpuName);
	tsinfo("CPU cores: %", inf.cpuProcessorCount);

	switch (inf.cpuArchitecture)
	{
	case ECPUArchitecture::eCPUarchAMD64:
		tsinfo("CPU architecture: %", "AMD64");
		break;
	case ECPUArchitecture::eCPUarchX86:
		tsinfo("CPU architecture: %", "X86");
		break;
	case ECPUArchitecture::eCPUarchARM:
		tsinfo("CPU architecture: %", "ARM");
		break;
	default:
		tsinfo("CPU architecture: unknown");
	}

	switch (inf.cpuVendorID)
	{
	case ECpuVendorID::eCpuAMD:
		tsinfo("CPU vendor: %", "AMD");
		break;
	case ECpuVendorID::eCpuIntel:
		tsinfo("CPU vendor: %", "Intel");
		break;
	default:
		tsinfo("CPU vendor: unknown");
	}

	//Other
	tsinfo("No. displays: %", inf.numDisplays);
	tsinfo("OS name: %", inf.osName);

	//Memory information
	SSystemMemoryInfo meminf;
	getSystemMemoryInformation(meminf);

	tsinfo("Memory capacity: %MB", (meminf.mCapacity / (1024ui64 * 1024ui64)));

	tsinfo("");
	tsinfo("Hello %", inf.userName);
}