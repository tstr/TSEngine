/*
	Utility header -

	Contains a set of useful platform functions
*/

#pragma once

#include <C3E\core\corecommon.h>
#include <string>

namespace C3E
{
	//Get number of logical cores
	C3E_CORE_API size_t GetProcessorCount();

	C3E_CORE_API std::string GetProcessorName();

	//in kilobytes(kB)
	C3E_CORE_API size_t GetMaxMemoryCapacity();

	//Get size of working set in kilobytes(kB)
	C3E_CORE_API size_t GetMemoryUsage();

	//Get monitor dimensions
	C3E_CORE_API size_t GetMonitorSizeX();
	C3E_CORE_API size_t GetMonitorSizeY();

	C3E_CORE_API std::string GetExcecutableDirectory();

	//Clipboard functions
	C3E_CORE_API void CopyToClipboard(const char*);
	C3E_CORE_API std::string CopyFromClipboard();

	//Show dialog box
	C3E_CORE_API int MsgBox(const char* text, const char* header = "");

	C3E_CORE_API void GenerateMinidump(const char* fn);
	C3E_CORE_API std::string PrintCallstack();
}