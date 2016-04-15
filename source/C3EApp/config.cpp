/*
	Configuration handling class source
*/

#include "config.h"
#include <C3E\filesystem\filexml.h>
#include <regex>

using namespace std;
using namespace C3E;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Config class
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Config::Impl
{
	FileSystem m_fs;
	unique_ptr<FileSystem::IXMLfile> m_fsxml;

	bool m_modified = false;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Config::Config() : pImpl(new Impl)
{

}

Config::Config(const char* file) : pImpl(new Impl)
{
	Open(file);
}

Config::~Config()
{
	Close();
}

bool Config::Open(const char* cfg)
{
	if (!FileSystem::FileExists(cfg))
	{
		ofstream f(cfg);
	}

	pImpl->m_fsxml = move(pImpl->m_fs.OpenXMLfile(cfg));
	pImpl->m_modified = false;

	return (pImpl->m_fsxml.get()) ? true : false;
}

const char* Config::FindEntry(const char* path, const char* def)
{
	regex rgx("(\\w+)");

	string entry(path);
	auto& xml = pImpl->m_fsxml;

	regex_iterator<string::iterator> iter(entry.begin(), entry.end(), rgx);

	FileSystem::IXMLnode* node = pImpl->m_fsxml->Root();

	bool modified = false;

	for (; iter != regex_iterator<string::iterator>(); iter++)
	{
		auto _node = node->GetNode(iter->str().c_str());
		
		if (!_node)
		{
			_node = xml->AllocateNode(iter->str().c_str());
			node->AddNode(_node);
			pImpl->m_modified = true;
			modified = true;
		}

		node = _node;
	}

	if (!node || (node->Value() == "") || modified)
	{
		node->SetValue(def);
		pImpl->m_modified = true;

		return def;
	}

	return node->Value();
}

void Config::Close()
{
	if (pImpl->m_modified)
	{
		pImpl->m_fsxml->Save();
	}

	pImpl->m_fsxml.reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Command line class
///////////////////////////////////////////////////////////////////////////////////////////////////////////

CommandLine::CommandLine(const char* cmd_line) :
	m_buffer(cmd_line)
{
	if (m_buffer != "")
	{
		int start_pos = 0;
		int pos = 0;

		bool mode = true;

		for (const char& i : m_buffer)
		{
			if (i == '-')
			{
				mode = !mode;

				if (mode) //parse
				{
					m_command_table.push_back(m_buffer.substr(start_pos + 1, (pos - 1 - start_pos)));
				}

				start_pos = pos;
			}
			else if ((pos + 1) == m_buffer.size())
			{
				m_command_table.push_back(m_buffer.substr(start_pos + 1, (pos - start_pos)));
			}

			pos++;
		}

		if (!m_command_table.empty())
			for (string& c : m_command_table)
			{
				for (char& ch : c)
				{
					tolower(ch);
				}
			}
	}
}

bool CommandLine::ArgumentTagExists(const char* tag)
{
	regex rgx("(\\w+)");

	for (string& cmd : m_command_table)
	{
		regex_iterator<string::iterator> iter(cmd.begin(), cmd.end(), rgx);

		if (iter != regex_iterator<string::iterator>())
		{
			if (iter->str() == tag)
				return true;
		}
	}

	return false;
}

CommandLine::Argument CommandLine::GetArgument(const char* tag)
{
	regex rgx("(\\w+)");

	for (string& cmd : m_command_table)
	{
		regex_iterator<string::iterator> iter(cmd.begin(), cmd.end(), rgx);

		if (iter != regex_iterator<string::iterator>())
		{
			if (iter->str() == tag)
			{
				return ParseArgument(cmd.c_str());
			}
		}
	}

	return Argument();
}

CommandLine::Argument CommandLine::ParseArgument(const char* str)
{
	string buffer(str);

	size_t pos = buffer.find_first_of('=');

	Argument arg;

	if (pos == string::npos)
	{
		regex rgx("(\\w+)");
		regex_iterator<string::iterator> iter(buffer.begin(), buffer.end(), rgx);
		if (iter != regex_iterator<string::iterator>())
		{
			arg = Argument(iter->str().c_str(), "");
		}
	}
	else
	{
		string buffer0(buffer.substr(0, pos));
		string buffer1(buffer.substr(pos + 1, buffer.size() - 1));

		while (buffer0.front() == ' ') { buffer0.erase(0, 1); } while (buffer1.front() == ' ') { buffer1.erase(0, 1); }
		while (buffer0.back() == ' ') { buffer0.erase(buffer0.size() - 1, 1); } while (buffer1.back() == ' ') { buffer1.erase(buffer1.size() - 1, 1); }

		arg = Argument(buffer0.c_str(), buffer1.c_str());
	}

	return arg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////