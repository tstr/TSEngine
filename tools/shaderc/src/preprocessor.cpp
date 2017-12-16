/*
	Preprocessor source

	todo:
	 - multiline macros
	 - macro arguments
	 - "if" and "elif" preprocessor commands
	 - prevent infinite macro recursion
	 - handle syntax errors properly
*/

#pragma once

#include "preprocessor.h"

#include <tscore/pathutil.h>
#include <tscore/debug/log.h>

#include <fstream>
#include <regex>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Helper function

	Strip C style comments from a stream (input) and write result to stream (output)
*/
static void stripComments(istream& input, ostream& output);

/////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Preprocess a given file
*/
EPreprocessorStatus CPreprocessor::process(const Path& filepath, ostream& outstream)
{
	//Open the file stream
	stringstream text;
	
	//If comment stripping is enabled
	if (m_stripComments)
	{
		//Strip comments from input before preprocessing
		stripComments(ifstream(filepath.str()), text);
	}
	else
	{
		//Otherwise just preprocess raw input
		text << ifstream(filepath.str()).rdbuf();
	}
	
	if (text.fail())
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
	String linebuf;
	String lineraw;
	while (getline(text, lineraw))
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
				if (speechPosStart == speechPosEnd || speechPosStart == String::npos || speechPosEnd == String::npos)
				{
					//If normal speech marks are not present then search for angled braces instead - <>
					speechPosStart = linebuf.find_first_of('<');
					speechPosEnd = linebuf.find_last_of('>');

					//If the angled braces are not present either then the include statement is invalid
					if (speechPosStart == speechPosEnd || speechPosStart == String::npos || speechPosEnd == String::npos)
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
				size_t firstpos = min(linebuf.find_first_not_of(' ', linebuf.find_first_of(' ')), linebuf.find_first_not_of('\t', linebuf.find_first_of('\t')));

				//The define command must have a macro name as an argument - return failure
				if (firstpos == string::npos)
				{
					tswarn("preprocessor command \"define\" expects an argument");
					return ePreprocessStatus_SyntaxError;
				}

				//Find second whitespace character
				size_t secpos = min(linebuf.find_first_of(' ', firstpos), linebuf.find_first_of('\t', firstpos));

				SPreprocessorMacro macro;
				macro.name = linebuf.substr(firstpos, secpos - firstpos);

				if (secpos != string::npos)
					macro.value = linebuf.substr(secpos + 1); //only set the macro value if the statement defines one

				m_macroTable[macro.name] = trim(macro.value);
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
				outstream << evaluateText(lineraw);
				outstream << endl;
			}
		}
	}

	return ePreprocessStatus_Ok;
}

//Replace "from" in "str" with "to"
bool replace(String& str, const String& from, const String& to)
{
	size_t start_pos = str.find(from);

	if (start_pos == String::npos)
		return false;

	str.replace(start_pos, from.length(), to);
	
	return true;
};

// todo: reimplement: this is a very broken way of evaluating macros,
// will not work if the same macro appears twice in the same string
String CPreprocessor::evaluateText(const String& text)
{
	//Result buffer
	string buf(text);

	//Identifier regex
	static const regex rgx("([a-zA-Z_][a-zA-Z0-9_]*)");

	using Iter = regex_iterator<string::const_iterator>;

	for (Iter it(text.begin(), text.end(), rgx); it != Iter(); it++)
	{
		auto macroIt = m_macroTable.find(it->str());

		if (macroIt != m_macroTable.end())
		{
			//Search and replace again
			//Workaround because regex_replace doesn't have a custom replacer predicate
			replace(buf, it->str(), evaluateText(macroIt->second));
		}
	}

	return buf;
}

void stripComments(istream& input, ostream& output)
{
	//Commenting states, mutually exclusive
	bool in_block_comment = false;
	bool in_line_comment = false;

	//Having the prev char be the cur char is a bit counter intuitive but it works
	char cur = 0;

	while (input)
	{
		//The next char is actually the current char
		//We look ahead by one
		char nxt = 0;
		input.read(&nxt, 1);

		//If in commenting state
		if (in_block_comment || in_line_comment)
		{
			//Detect end of comment depending on current comment type

			if (in_block_comment)
			{
				//If end of block comment
				if ((cur == '*') && (nxt == '/'))
				{
					//Reset to writing state
					in_block_comment = false;
					nxt = 0;
				}
			}
			else if (in_line_comment)
			{
				//If end of line comment
				if (nxt == '\n')
				{
					//Reset to writing state
					in_line_comment = false;
					nxt = 0;
				}
			}
		}
		//Otherwise if in writing state
		else
		{
			//If current char marks start of comment
			if (cur == '/')
			{
				//Determine comment type
				if (nxt == '*')
				{
					in_block_comment = true;
					cur = 0;
				}
				else if (nxt == '/')
				{
					in_line_comment = true;
					cur = 0;
				}

				//If nxt char doesn't match start of comment
				//just continue writing to the output as normal
			}

			//If not in commenting state
			//And current char isn't null
			if (!in_block_comment && !in_line_comment && cur != 0)
				//Write to output
				output.write(&cur, 1);
		}

		//Current is next
		cur = nxt;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

const char* CPreprocessor::getPreprocessorStatusString(EPreprocessorStatus status)
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
