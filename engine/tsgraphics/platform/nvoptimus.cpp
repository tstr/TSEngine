/*
	The global variable "NvOptimusEnablement" forces systems using Nvidia Optimus to enable the high performance gpu
*/

#include <Windows.h>

extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}