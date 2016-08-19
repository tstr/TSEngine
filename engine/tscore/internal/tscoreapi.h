/*
	TSCORE library import/export macro
*/

#pragma once

#include <tsconfig.h>


#if defined (_WIN32) 

	#ifdef TS_BUILD_SHARED_LIBRARIES

		#if defined(cmakelib_EXPORTS)
		
			#define TSCORE_API __declspec(dllexport)
			
		#else
			
			#define TSCORE_API __declspec(dllimport)
			
		#endif
	
	#else

		#define TSCORE_API

	#endif
	
#else
	
	#define TSCORE_API
	
#endif

