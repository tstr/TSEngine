/*
	Config file parser source
*/

#include "configfile.h"
#include <fstream>
#include <tscore/debug/log.h>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isWhitespace(char c)
{
	return (c == ' ' || c == '\t');
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigFile::parseSection(const std::string& line, ConfigFile::Section& section)
{
	size_t pos0 = line.find_first_of('[');
	size_t pos1 = line.find_first_of(']');

	if (pos1 == string::npos)
		return false;

	if (pos0 == string::npos)
		return false;

	if (pos1 < pos0)
		return false;

	string name(line.substr(pos0 + 1, pos1 - pos0 - 1));
	trim(name);

	if (name == "")
	{
		return false;
	}

	toLower(name);

	swap(section, name);

	return true;
}

bool ConfigFile::parseProperty(const std::string& line, ConfigFile::SProperty& property)
{
	auto v = split(line, '=');

	if (v.size() != 2)
	{
		return false;
	}

	property.key = move(v[0]);
	property.value = move(v[1]);

	property.key = trim(property.key);
	property.value = trim(property.value);

	toLower(property.key);
	toLower(property.value);

	return true;
}

void ConfigFile::trimComments(std::string& line)
{
	size_t pos = line.find_first_of('#');
	if (pos != string::npos)
		line.erase(pos, string::npos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Parse a config file and loads it's contents into memory
bool ConfigFile::load(const ts::Path& configpath)
{
	ifstream filestream(configpath.str());

	if (filestream.good())
	{
		m_configpath = configpath;

		//Copy entire filestream into memory
		stringstream stream;
		stream << filestream.rdbuf();
		filestream.close();

		string line;
		size_t linenumber = 0;

		bool in_section = false;
		Section currentsection;

		//Reads each line each iteration
		while (getline(stream, line))
		{
			linenumber++;

			//Remove characters preceded by the # character
			trimComments(line);
			//Remove whitespace
			line = trim(line);

			if (line.size() > 0)
			{
				//Is section
				if (line.front() == '[')
				{
					Section section;
					if (!parseSection(line, section))
					{
						tswarn("Unable to parse section - line %", linenumber);
						continue;
					}

					in_section = true;
					currentsection = move(section);
				}
				//Is property
				else
				{
					if (!in_section)
					{
						tswarn("Property is not part of section - line %", linenumber);
						continue;
					}

					SProperty property;
					if (!parseProperty(line, property))
						tswarn("Unable to parse property - line %", linenumber);

					m_properties.insert(make_pair(currentsection, property));
				}
			}

			line.clear();
		}

		return true;
	}
	else
	{
		tserror("Config file - '%' does not exist", configpath.str());
		return false;
	}
}

ConfigFile::~ConfigFile()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t ConfigFile::getSectionPropertyCount(const Section& section) const
{
	string key(section);
	toLower(key);
	return m_properties.count(key);
}

size_t ConfigFile::getSectionCount() const
{
	size_t sz = 0;
	for (auto& i : m_properties)
	{
		sz++;
	}
	return sz;
}


void ConfigFile::getSectionProperties(const Section& section, SPropertyArray& properties)
{
	//Section key
	string key(section);
	toLower(key);
	//Get iterator pair for this section
	auto range = m_properties.equal_range(key);

	properties.clear();

	if (range.first == range.second)
	{
		return;
	}

	properties.reserve(getSectionPropertyCount(section));

	//Iterate over section
	for (auto it = range.first; it != range.second; it++)
	{
		properties.push_back(it->second);
	}
}

bool ConfigFile::getProperty(const Section& section, SProperty& property)
{
	//Section key
	string key(section);
	toLower(key);
	//Get iterator pair for this section
	auto range = m_properties.equal_range(key);
	
	if (range.first == range.second)
	{
		return false;
	}

	//Iterate over section
	for (auto it = range.first; it != range.second; it++)
	{
		//Compare property keys
		if (compare_string_weak(it->second.key, property.key))
		{
			property.value = it->second.value;
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////