/*
	INI file parser source
*/

#include "INIReader.h"
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

bool INIReader::parseSection(const std::string& line, INIReader::Section& section)
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

bool INIReader::parseProperty(const std::string& line, Property& property)
{
	auto v = split(line, '=');

	if (v.size() != 2)
	{
		return false;
	}

	Property prop;

	prop.key = move(v[0]);
	prop.value = move(v[1]);

	if (prop.key.find('.') != string::npos)
		return false;

	prop.key = trim(prop.key);
	prop.value = trim(prop.value);

	toLower(prop.key);
	toLower(prop.value);

	property = move(prop);

	return true;
}

void INIReader::trimComments(std::string& line)
{
	size_t pos = line.find_first_of('#');
	if (pos != string::npos)
		line.erase(pos, string::npos);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Parse a config file and loads it's contents into memory
bool INIReader::load(const ts::Path& configpath)
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

					Property property;
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

INIReader::~INIReader()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool INIReader::isSection(const Section& section)
{
	string key(section);
	toLower(key);
	return (m_properties.count(key) > 0);
}

size_t INIReader::getSectionPropertyCount(const Section& section) const
{
	string key(section);
	toLower(key);
	return m_properties.count(key);
}

size_t INIReader::getSectionCount() const
{
	size_t sz = 0;
	for (auto& i : m_properties)
	{
		sz++;
	}
	return sz;
}


void INIReader::getSectionProperties(const Section& section, PropertyArray& properties)
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


void INIReader::getSections(SectionArray& sections)
{
	for (auto it = m_properties.begin(), end = m_properties.end(); it != end; it = m_properties.upper_bound(it->first))
	{
		sections.push_back(it->first);
	}
}

bool INIReader::getProperty(const PropertyKey& keystr, PropertyValue& valuestr)
{
	size_t splitpos = keystr.find_last_of('.');

	//Get section key and property key from keystr
	string sectionkey(keystr.substr(0, splitpos));
	string propertykey(keystr.substr(splitpos + 1));
	toLower(sectionkey);
	toLower(propertykey);

	//Get iterator pair for this section
	auto range = m_properties.equal_range(sectionkey);
	
	if (range.first == range.second)
	{
		return false;
	}

	//Iterate over section
	for (auto it = range.first; it != range.second; it++)
	{
		//Compare property keys
		if (compare_string_weak(it->second.key, propertykey))
		{
			valuestr = it->second.value;
			break;
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////