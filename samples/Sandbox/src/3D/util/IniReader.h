/*
	INI file parser header

	Format:

		[Section]
		key = value 
*/

#pragma once

#include <tsengine/abi.h>
#include <tscore/path.h>
#include <tscore/strings.h>
#include <tscore/types.h>
//stdlib
#include <map>
#include <vector>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class INIReader
	{
	public:

		typedef std::string Section;
		typedef std::string PropertyKey;
		typedef std::string PropertyValue;

		struct Property
		{
			PropertyKey key;
			PropertyValue value;
		};

		typedef std::vector<Property> PropertyArray;
		typedef std::vector<Section> SectionArray;

		bool load(const ts::Path& configpath);
		void reload() { load(m_configpath); }

		Path getPath() const { return m_configpath; }

		INIReader() {}
		INIReader(const ts::Path& configpath) { load(configpath); }
		~INIReader();

		void getSectionProperties(const Section& section, PropertyArray& properties);
		void getSections(SectionArray& sections);

		size_t getSectionPropertyCount(const Section& section) const;
		size_t getSectionCount() const;
		bool isSection(const Section& section);
		
		bool getProperty(const PropertyKey& key, PropertyValue& val);
		
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
		std::multimap<Section, Property> m_properties;

		bool parseSection(const std::string& line, Section& section);
		bool parseProperty(const std::string& line, Property& property);

		void trimComments(std::string& line);
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////s