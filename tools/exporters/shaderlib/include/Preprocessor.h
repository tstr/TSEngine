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
#include <tscore/pathutil.h>

#include <unordered_map>
#include <vector>
#include <stack>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	typedef String PreprocessorString;

	struct SPreprocessorMacro
	{
		PreprocessorString name;
		PreprocessorString value;
	};

	typedef std::vector<SPreprocessorMacro> PreprocessorMacroList;
	
	enum EPreprocessorStatus
	{
		ePreprocessStatus_Ok				= 0,
		ePreprocessStatus_Fail				= 1,
		ePreprocessStatus_UnknownStatement	= 2,
		ePreprocessStatus_FileNotFound		= 3,
		ePreprocessStatus_SyntaxError		= 4
	};
	
	class CPreprocessor
	{
	private:
		
		std::unordered_map<PreprocessorString, PreprocessorString> m_macroTable;	//Table of macros name/value pairs
		std::vector<Path> m_includeDirs;											//List of include file directories to search through
		std::stack<bool> m_macroStateStack;											//Stack for tracking the states of nested conditional statements
		bool m_stripComments = true;

		String evaluateText(const String& text);

		bool getMacroValue(const PreprocessorString& name, PreprocessorString& value)
		{
			//Look up macro name in table
			auto it = m_macroTable.find(name);

			//Check if iterator is valid
			if (it != m_macroTable.end())
			{
				value = it->second;
				return true;
			}

			return false;
		}

		inline bool isMacro(const PreprocessorString& name)
		{
			return (m_macroTable.find(name) != m_macroTable.end());
		}

		inline bool isStatement(const String& statement)
		{
			return (!statement.empty() && statement.front() == '#');
		}

		inline bool isCommand(const String& statement, const String& command)
		{
			return statement.find(command) == 1;
		}

		static const char* getPreprocessorStatusString(EPreprocessorStatus status);

	public:

		/*
			Add an include directory to the search path
		*/
		bool addIncludePath(const Path& includeDir)
		{
			if (isDirectory(includeDir))
			{
				m_includeDirs.push_back(includeDir);
				return true;
			}
		}

		/*
			Add a predefined macro
		*/
		void addMacro(const SPreprocessorMacro& macro)
		{
			m_macroTable[macro.name] = macro.value;
		}

		/*
			Set whether or not to strip comments from the output
		*/
		void setCommentStrip(bool enable)
		{
			m_stripComments = enable;
		}
		
		EPreprocessorStatus process(const Path& filepath, std::ostream& out);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
