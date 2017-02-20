/*
	Helper functions for printing information about the application and the system
*/

#include "appinfo.h"
#include <tsversion.h>
#include <tscore/system/info.h>
#include <tscore/debug/log.h>

namespace ts
{
	void printSystemInfo()
	{
		//CPU information
		SSystemInfo inf;
		getSystemInformation(inf);
		
		tsinfo("CPU name: %", inf.cpuName);
		tsinfo("CPU cores: %", inf.cpuProcessorCount);

		switch (inf.cpuArchitecture)
		{
		case ECPUArchitecture::eCPUarchAMD64:
			tsinfo("CPU architecture: AMD64");
			break;
		case ECPUArchitecture::eCPUarchX86:
			tsinfo("CPU architecture: X86");
			break;
		case ECPUArchitecture::eCPUarchARM:
			tsinfo("CPU architecture: ARM");
			break;
		default:
			tsinfo("CPU architecture: unknown");
		}

		switch (inf.cpuVendorID)
		{
		case ECpuVendorID::eCpuAMD:
			tsinfo("CPU vendor: AMD");
			break;
		case ECpuVendorID::eCpuIntel:
			tsinfo("CPU vendor: Intel");
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
	
	void printRepositoryInfo()
	{
		tsinfo("Git refspec: %", TS_GIT_REFSPEC);
		tsinfo("Git commit SHA1: %", TS_GIT_SHA1);
		tsinfo("Git commit date: \"%\"", TS_GIT_COMMIT_DATE);
		tsinfo("Git commit subject: \"%\"", TS_GIT_COMMIT_SUBJECT);
	}
}
