/*
	Config file parser header

	Config files use a simple INI format, eg.

	[Section]
	key = value 
*/

#pragma once

#include <tscore/filesystem/path.h>
#include <tscore/strings.h>
#include <tscore/types.h>
//stdlib
#include <map>
#include <vector>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct SConfigProperty
	{
		std::string section;
		std::string keyname;
		std::string keyvalue;
	};
	
	class ConfigFile
	{
	public:

		struct SProperty
		{
			std::string key;
			std::string value;
		};

		typedef std::string Section;
		typedef std::vector<SProperty> SPropertyArray;

		bool load(const ts::Path& configpath);
		void reload() { load(m_configpath); }

		Path getPath() const { return m_configpath; }

		ConfigFile() {}
		ConfigFile(const ts::Path& configpath) { load(configpath); }
		~ConfigFile();

		void getSectionProperties(const Section& section, SPropertyArray& properties);
		size_t getSectionPropertyCount(const Section& section) const;
		size_t getSectionCount() const;
		
		bool getProperty(const Section& section, SProperty& property);
		
		template<
			typename t,
			class = std::enable_if<std::is_integral<t>::value>::type
		>
		inline bool getProperty(
			const Section& section,
			const std::string& key,
			t& value
		)
		{
			using namespace ts;
			SProperty prop;
			prop.key = key;
			if (getProperty(section, prop))
			{
				std::stringstream stream;
				stream << prop.value;
				stream >> value;

				return true;
			}
			else
			{
				return false;
			}
		}

	private:

		Path m_configpath;
		std::multimap<Section, SProperty> m_properties;

		bool parseSection(const std::string& line, Section& section);
		bool parseProperty(const std::string& line, SProperty& property);

		void trimComments(std::string& line);
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////s