/*
	Simple C-style preprocessor

	The function "preprocessFile" takes a filename as input as well as a few optional arguments and parses the file,
	the result is written to an std::ostream object.

	Preprocessor commands supported are:

		- define
		- undef
		- include
		- ifdef
		- endif
		- else

	(preprocessor commands must be written in lowercase)

*/

#pragma once

#include <tscore/strings.h>
#include <tscore/path.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum { ePreprocessorMacroLength = 256 };

	typedef StaticString<ePreprocessorMacroLength> PreprocessorString;

	struct SPreprocessorMacro
	{
		PreprocessorString name;
		PreprocessorString value;
	};
	
	enum EPreprocessorStatus
	{
		ePreprocessStatus_Ok				= 0,
		ePreprocessStatus_Fail				= 1,
		ePreprocessStatus_UnknownStatement	= 2,
		ePreprocessStatus_FileNotFound		= 3,
		ePreprocessStatus_SyntaxError		= 4
	};
	
	EPreprocessorStatus preprocessFile(
		const Path& filepath,
		std::ostream& outputbuf,
		const SPreprocessorMacro* macros = nullptr,
		uint macroCount = 0,
		const Path* includeDirs = nullptr,
		uint includeDirCount = 0
	);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
