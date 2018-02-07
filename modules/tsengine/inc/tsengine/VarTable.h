/*
	Variable Table Header
*/

#pragma once

#include <tscore/types.h>
#include <tscore/maths.h>
#include <tscore/strings.h>

#include <map>
#include <unordered_map>
#include <mutex>

namespace ts
{
	class VarTable
	{
	private:

		typedef std::mutex Mutex;
		typedef std::lock_guard<Mutex> Guard;

	public:

		typedef String Name;
		
		struct Entry
		{
			Name name;
			Name value;
		};

		typedef std::vector<Entry> List;

		/*
			Get value of variable as a string
		*/
		bool get(Name name, String& val) const
		{
			toLower(name);

			Guard g(m_mutex);

			auto it = m_table.find(name);

			if (it != m_table.end())
			{
				val = it->second;
				return true;
			}

			return false;
		}
		
		/*
			Get value of variable as a generic type
		*/
		template<typename T>
		bool get(Name name, T& val) const
		{
			String valStr;

			if (get(name, valStr))
			{
				stringstream conv;
				conv << valStr;
				conv >> val;

				return true;
			}

			return false;
		}
		
		/*
			Get value of variable as a Vector
		*/
		bool get(Name name, Vector& val) const
		{
			String valStr;

			if (get(name, valStr))
			{
				auto tokens = split(valStr, ',');
				if (tokens.size() > 0) val.x() = stof(trim(tokens[0]));
				if (tokens.size() > 1) val.y() = stof(trim(tokens[1]));
				if (tokens.size() > 2) val.z() = stof(trim(tokens[2]));
				if (tokens.size() > 3) val.w() = stof(trim(tokens[3]));

				return true;
			}

			return false;
		}

		/*
			Set value of variable
		*/
		void set(Name name, const String& val)
		{
			toLower(name);
			Guard lk(m_mutex);
			m_table[name] = val;
		}

		/*
			Set value of Vector variable
		*/
		void set(Name name, const Vector& val)
		{
			std::stringstream ss;
			ss << val.x() << ", ";
			ss << val.y() << ", ";
			ss << val.z() << ", ";
			ss << val.w();

			set(name, ss.str());
		}

		/*
			Set value of generic variable
		*/
		template<typename T>
		void set(Name name, const T& val)
		{
			set(name, std::to_string(val));
		}
		
		/*
			Check if variable of given name exists in table
		*/
		bool exists(Name name) const
		{
			toLower(name);
			
			Guard g(m_mutex);

			return m_table.find(name) != m_table.end();
		}
		
		/*
			Get List of variable name/values in table
		*/
		void toList(List& ls) const
		{
			ls.clear();

			Guard g(m_mutex);

			for (auto p : m_table)
			{
				ls.push_back({ p.first, p.second });
			}
		}

		List toList() const
		{
			List l;
			toList(l);
			return std::move(l);
		}
		
	private:
		
		mutable std::mutex m_mutex;
		std::unordered_map<Name, Name> m_table;
	};
}
