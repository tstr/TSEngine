/*
	Core engine defines
*/

#pragma once

#define IMPORT_ATTRIB __declspec(dllimport)
#define EXPORT_ATTRIB __declspec(dllexport)

#ifdef CT_USE_DYNAMIC_LIB

#ifndef CT_CORE_API
#define CT_CORE_API IMPORT_ATTRIB
#endif

#else

#ifndef CT_CORE_API
#define CT_CORE_API
#endif

#endif

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef _WIN32
#define LINK_LIB(x) __pragma(comment(lib, x))
#else
#define LINK_LIB(x)
#endif

#include <ct\core\assert.h>

//////////////////////////////////////////////////////////////////////////////////////////////

namespace CT
{
	typedef char int8;
	typedef short int16;
	typedef int int32;
	typedef long long int64;

	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned int uint32;
	typedef unsigned long long uint64;

	typedef float float32;
	typedef double float64;

	typedef wchar_t wchar;
	typedef unsigned char byte;


	inline const char* nullstr()
	{
		static const char str[] = "\0";
		return str;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////