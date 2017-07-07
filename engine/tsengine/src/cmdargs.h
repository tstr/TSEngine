/*
	Commandline argument parser header
*/

#pragma once

#include <tsengine/abi.h>
#include <unordered_map>
#include <tscore/types.h>
#include <tscore/strings.h>

namespace ts
{
	class CommandLineArgs
	{
	private:
		
		String m_commandLine;
		std::unordered_map<std::string, std::string> m_argPairs;
		
	public:
		
		CommandLineArgs() {}
		CommandLineArgs(const String& cmdline) { parse(cmdline); }
		~CommandLineArgs() {}
		
		CommandLineArgs(char** argv, int argc)
		{
			String cmdline;
			for (int i = 1; i < argc; i++)
			{
				cmdline += " ";
				cmdline += argv[i];
				cmdline += " ";
			}

			parse(cmdline);
		}
		
		void parse(const String& cmdline);
		
		bool isArgumentTag(const std::string& tag) const
		{
			return (m_argPairs.find(tag) != m_argPairs.end());
		}
		
		String getArgumentValue(const String& tag) const
		{
			auto it = m_argPairs.find(tag);
			
			if (it == m_argPairs.end())
				return "";
			
			return it->second;
		}

		size_t getArgumentCount() const
		{
			return m_argPairs.size();
		}

		String getArguments() const
		{
			return m_commandLine;
		}
	};
}