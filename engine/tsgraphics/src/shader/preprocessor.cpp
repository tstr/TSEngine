/*
	Preprocessor source

	todo:
	 - multiline macros
	 - macro arguments
	 - "if" and "elif" preprocessor commands
	 - prevent infinite macro recursion
	 - handle syntax errors properly
	 - remove comments from the output
*/

#pragma once

#include "preprocessor.h"

#include <tscore/filesystem/pathhelpers.h>
#include <tscore/debug/log.h>

#include <fstream>
#include <map>
#include <vector>
#include <stack>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

const char* getPreprocessorStatusString(EPreprocessorStatus status);

namespace std
{
	template<>
	struct less<PreprocessorString>
	{
		//Operator <
		bool operator()(const PreprocessorString& left, const PreprocessorString& right) const
		{
			return (strcmp(left.str(), right.str()) < 0);
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////

class CPreprocessor
{
private:
	
	map<PreprocessorString, PreprocessorString> m_macroTable;	//Table of macros name/value pairs
	vector<Path> m_includeDirs;									//List of include file directories to search through
	stack<bool> m_macroStateStack;								//Stack for tracking the states of nested conditional statements

	void evaluateText(string& text);

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

	inline bool isStatement(const string& statement)
	{
		return (!statement.empty() && statement.front() == '#');
	}

	inline bool isCommand(const string& statement, const string& command)
	{
		return statement.find(command) == 1;
	}

public:

	//Add an array of paths for the preprocessor to search for include files in
	void addIncludeDirs(const Path* includeDirs, uint includeDirCount)
	{
		if (includeDirs && includeDirCount)
			for (uint i = 0; i < includeDirCount; i++)
				m_includeDirs.push_back(includeDirs[i]);
	}

	//Add an array of macros for the preprocessor to use
	void addMacros(const SPreprocessorMacro* macros, uint macroCount)
	{
		if (macros && macroCount)
			for (uint i = 0; i < macroCount; i++)
				m_macroTable[macros[i].name] = macros[i].value;
	}
	
	int process(const Path& filepath, ostream& out);
};

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	EPreprocessorStatus preprocessFile(
		const Path& filepath,
		ostream& outputbuf,
		const SPreprocessorMacro* macros,
		uint macroCount,
		const Path* includeDirs,
		uint includeDirCount
	)
	{
		CPreprocessor pp;

		pp.addIncludeDirs(includeDirs, includeDirCount);
		pp.addMacros(macros, macroCount);

		return (EPreprocessorStatus)pp.process(filepath, outputbuf);
	}
}

//Preprocess a file
int CPreprocessor::process(const Path& filepath, ostream& outstream)
{
	//Open the file stream
	ifstream file(filepath.str());

	if (file.fail())
	{
		return ePreprocessStatus_FileNotFound;
	}

	//When false no lines are written to the output stream
	bool write_lines = true;
	//Counter for tracking nested conditional statements
	uint32 write_line_counter = 0;
	
	//Clear stack
	m_macroStateStack = stack<bool>();
	m_macroStateStack.push(true);		//current scope is visible so set root state to true

	//Iterate over each line in the file
	string linebuf;
	string lineraw;
	while (getline(file, lineraw))
	{
		linebuf = trim(lineraw);

		//If the first character is a pound sign then the line is a preprocessor command
		if (isStatement(linebuf))
		{
			//Check if preprocessor command is an include statement - ignore the first character in the string
			if (isCommand(linebuf, "include"))
			{
				if (!write_lines)
					continue;

				//If true then the include statement path is resolved from the user specified include path list
				bool needSearch = false;

				//Speech mark positions
				size_t speechPosStart = linebuf.find_first_of('\"');
				size_t speechPosEnd = linebuf.find_last_of('\"');

				//Check if speech marks are present - ""
				if (speechPosStart == speechPosEnd || speechPosStart == string::npos || speechPosEnd == string::npos)
				{
					//If normal speech marks are not present then search for angled braces instead - <>
					speechPosStart = linebuf.find_first_of('<');
					speechPosEnd = linebuf.find_last_of('>');

					//If the angled braces are not present either then the include statement is invalid
					if (speechPosStart == speechPosEnd || speechPosStart == string::npos || speechPosEnd == string::npos)
						return ePreprocessStatus_SyntaxError;

					needSearch = true;
				}

				Path includeStatementParam = linebuf.substr(speechPosStart + 1, speechPosEnd - (speechPosStart + 1));

				Path includeFilePath;

				if (needSearch)
				{
					if (m_includeDirs.empty())
						return ePreprocessStatus_FileNotFound;

					//Resolve a valid file path from the stored include file directories
					if (!resolveFile(includeStatementParam, includeFilePath, &m_includeDirs[0], (uint32)m_includeDirs.size()))
						return ePreprocessStatus_FileNotFound;
				}
				else
				{
					includeFilePath = Path(filepath.getParent());
					includeFilePath.addDirectories(includeStatementParam);
				}

				//Load and preprocess the include file
				auto status = (EPreprocessorStatus)this->process(includeFilePath, outstream);

				if (status)
				{
					tserror("preprocessor error: %", getPreprocessorStatusString(status));
					return status;
				}
			}
			//Check if preprocessor command is a define statement
			else if (isCommand(linebuf, "define"))
			{
				if (!write_lines)
					continue;

				//Find first whitespace character
				size_t firstpos = linebuf.find_first_not_of(' ', linebuf.find_first_of(' '));

				//The define command must have a macro name as an argument - return failure
				if (firstpos == string::npos)
				{
					tswarn("preprocessor command \"define\" expects an argument");
					return ePreprocessStatus_SyntaxError;
				}

				//Find second whitespace character
				size_t secpos = linebuf.find_first_of(' ', firstpos);

				SPreprocessorMacro macro;
				macro.name.set(linebuf.substr(firstpos, secpos - firstpos));

				if (secpos != string::npos)
					macro.value.set(linebuf.substr(secpos + 1)); //only set the macro value if the statement defines one

				m_macroTable[macro.name] = macro.value;
			}
			//Check if preprocessor command is a undefine statement
			else if (isCommand(linebuf, "undef"))
			{
				if (!write_lines)
					continue;

				//Find first whitespace character
				size_t pos = linebuf.find_first_of(' ', linebuf.find_first_not_of(' '));

				//The undefine command must have a macro name as an argument - return failure
				if (pos == string::npos)
				{
					tswarn("preprocessor command \"undef\" expects an argument");
					return ePreprocessStatus_SyntaxError;
				}

				PreprocessorString macroName = (linebuf.substr(pos + 1));

				//Search for macro name in macro list
				m_macroTable.erase(macroName);
			}

			//The following commands must be evaluated even if write_line is false
			else if (isCommand(linebuf, "ifdef"))
			{
				//Find first whitespace character
				size_t pos = linebuf.find_first_of(' ', linebuf.find_first_not_of(' '));

				//The ifdef command must have a macro name as an argument - return failure
				if (pos == string::npos)
					return ePreprocessStatus_SyntaxError;

				PreprocessorString macroName = (linebuf.substr(pos + 1));
				
				//Get state of current scope
				bool currentState = m_macroStateStack.top();
				//Set child scope state
				m_macroStateStack.push(isMacro(macroName));
				//Only write lines to output if 
				if (currentState)
					write_lines = m_macroStateStack.top();
			}
			else if (isCommand(linebuf, "endif"))
			{
				//Get state of current scope
				bool currentState = m_macroStateStack.top();
				m_macroStateStack.pop();
				bool parentState = m_macroStateStack.top();

				//Check state of parent scope
				if (parentState && !currentState)
					write_lines = true;
			}
			else if (isCommand(linebuf, "else"))
			{
				//Get state of current scope
				bool currentState = m_macroStateStack.top();
				m_macroStateStack.pop();
				bool parentState = m_macroStateStack.top();

				if (parentState)
					write_lines = !currentState; //toggle state

				//Readd current state to stack
				m_macroStateStack.push(!currentState);
			}
			else
			{
				tswarn("unknown preprocessor command in statement: \"%\"", linebuf);
			}

			//todo: other preprocessor commands
			//else if (...)
		}
		else
		{
			if (write_lines)
			{
				evaluateText(lineraw);
				outstream << lineraw << endl;
			}
		}
	}

	return ePreprocessStatus_Ok;
}

void CPreprocessor::evaluateText(string& text)
{
	string token;

	ptrdiff textOffset = 0; //Start index of the token
	ptrdiff textIdx = 0;	  //Current index

	//Iterate over string which is to be evaluated
	for (; textIdx < (ptrdiff)text.size(); textIdx++)
	{
		char c = text[textIdx];

		//Check if char is a number or letter
		if (isdigit(c) || isalpha(c))
		{
			token += c; //add char to token
		}
		else
		{
			PreprocessorString macroValue;

			if (getMacroValue(token, macroValue))
			{
				//Save length of section being replaced and section that is replacing it
				ptrdiff oldlen = (ptrdiff)(textIdx - textOffset) - 1;
				ptrdiff newlen = (ptrdiff)macroValue.length();
				//Difference between lengths is the amount each index needs to be incremented by
				ptrdiff deltalen = newlen - oldlen;

				//Construct string of macro value and evaluate recursively
				string valstr(macroValue.str());
				evaluateText(valstr);

				text.replace(textOffset + 1, oldlen, valstr);

				//Increment by new difference
				textOffset += deltalen;
				textIdx += deltalen;
			}

			textOffset = textIdx;

			token.clear();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static const char* getPreprocessorStatusString(EPreprocessorStatus status)
{
	switch (status)
	{
	case (ePreprocessStatus_Ok):
	{
		static char s[] = "ok";
		return s;
	}
	case (ePreprocessStatus_Fail):
	{
		static char s[] = "fail";
		return s;
	}
	case (ePreprocessStatus_UnknownStatement):
	{
		static char s[] = "unknown statement";
		return s;
	}
	case (ePreprocessStatus_FileNotFound):
	{
		static char s[] = "file not found";
		return s;
	}
	default:
	{
		static char s[] = "unknown enum";
		return s;
	}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
