/*
	Commandline argument parsers source
*/

#pragma once

#include "cmdargs.h"

#include <tscore/debug/log.h>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandLineArgs::parse(const std::string& cmdline)
{
	m_commandLine = cmdline;

	//split command line into list of argument pairs
	auto list = split(m_commandLine, '-');

	for (const string& token : list)
	{
		string s(trim(token));

		if (s == "")
			continue;

		//split argument pair on first whitespace
		size_t pos = s.find_first_of(' ');

		//if argument is not a pair
		if (pos == string::npos)
		{
			m_argPairs[s.substr(0, pos)] = "";
		}
		else
		{
			m_argPairs[s.substr(0, pos)] = s.substr(pos + 1, s.size() - (pos + 1));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

