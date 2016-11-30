/*
	Config file parser header

	Config files use a simple INI format, eg.

	[Section]
	key = value 
*/

#pragma once

#include <tsengine/abi.h>
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

		typedef std::string Section;
		typedef std::string PropertyKey;
		typedef std::string PropertyValue;

		struct SProperty
		{
			PropertyKey key;
			PropertyValue value;
		};

		typedef std::vector<SProperty> SPropertyArray;

		bool TSENGINE_API load(const ts::Path& configpath);
		void reload() { load(m_configpath); }

		Path getPath() const { return m_configpath; }

		ConfigFile() {}
		ConfigFile(const ts::Path& configpath) { load(configpath); }
		TSENGINE_API ~ConfigFile();

		TSENGINE_API void getSectionProperties(const Section& section, SPropertyArray& properties);
		TSENGINE_API size_t getSectionPropertyCount(const Section& section) const;
		TSENGINE_API size_t getSectionCount() const;
		TSENGINE_API bool isSection(const Section& section);
		
		TSENGINE_API bool getProperty(const PropertyKey& key, PropertyValue& val);
		
		template<
			typename t,
			class = std::enable_if<!std::is_same<t, std::string>::value>::type
		>
		inline bool getProperty(
			const PropertyKey& key,
			t& value
		)
		{
			using namespace ts;

			PropertyValue valstr;

			if (getProperty(key, valstr))
			{
				std::stringstream stream;
				stream << valstr;
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

		TSENGINE_API bool parseSection(const std::string& line, Section& section);
		TSENGINE_API bool parseProperty(const std::string& line, SProperty& property);

		TSENGINE_API void trimComments(std::string& line);
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////s