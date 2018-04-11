/*
	Shader Annotation

	Annotations can be used to attach metadata to shader source code

	They can be attached to:
		- functions
		- structs
		- cbuffers

	An annotation consists of a name and a list of values:

	[name(value1,value2,...valueN)]
*/

#pragma once

#include <tscore/types.h>
#include <tscore/strings.h>
#include <vector>

namespace ts
{
	/*
		Shader annotation class

		Represents a single annotation name with optional values
	*/
	class ShaderAnnotation
	{
	private:

		String m_name;
		//vector<variant<string,number>> m_values; //todo: Annotation values

	public:

		ShaderAnnotation(const String& name) :
			m_name(name)
		{}

		// Get atrribute name
		const String& getName() const { return m_name; }

		// Get Annotation value
		//template<typename Value_t>
		//Value_t get(size_t idx) { ... }

		/*
			Comparison operators
		*/
		bool operator==(const String& attribName) const { return this->m_name == attribName; }
		bool operator==(const ShaderAnnotation& attrib) const { *this == attrib.getName(); }
	};

	/*
		Represents a set of annotations uniquely identified by name
	*/
	class ShaderAnnotationSet
	{
	private:

		std::vector<ShaderAnnotation> m_annotations;

	public:

		/*
			Add an annotation to the set (if it does not already exist)
		*/
		void add(const ShaderAnnotation& ant)
		{
			//If Annotation doesn't already exist
			if (!has(ant.getName()))
			{
				return m_annotations.push_back(ant);
			}
		}

		void remove(const String& antName)
		{
			auto it = find(m_annotations.begin(), m_annotations.end(), antName);

			if (it != m_annotations.end())
			{
				m_annotations.erase(it);
			}
		}

		const ShaderAnnotation& get(const String& antName) const
		{
			return *find(m_annotations.begin(), m_annotations.end(), antName);
		}
		
		bool has(const String& antName)
		{
			return find(m_annotations.begin(), m_annotations.end(), antName) != m_annotations.end();
		}

		void clear()
		{
			m_annotations.clear();
		}

		size_t count() const { m_annotations.size(); }
	};
}
