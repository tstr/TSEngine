/*
	String header - contains helpers for strings
*/

#pragma once

#include <tscore/abi.h>

#include <algorithm>

#include "types.h"
#include <sstream>
#include <vector>
#include <algorithm>

#define tocstr(x) std::to_string((x)).c_str()

//This is so std::to_string doesn't throw an error when you pass a string to it
namespace std
{
	inline std::string to_string(const std::string& str)
	{
		return str;
	}

	inline std::string to_string(const char* str)
	{
		using namespace std;
		return string(str);
	}

	inline std::string to_string(char c)
	{
		char str[] = { c, '\0' };
		return str;
	}
}

namespace ts
{
	typedef std::string String;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Format string - allows arguments to be inserted to locations in the string marked by the '%' char
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Format no arguments
	inline String format(const String& str)
	{
		return str;
	}

	//Format at least one argument - cstring
	template<typename arg_t, typename ... args_t>
	static String format(
		const char* str,
		arg_t&& arg,
		args_t&& ... args
		)
	{
		using namespace std;

		size_t sz = strlen(str);
		for (size_t i = 0; i < sz; i++)
		{
			if (str[i] == '%')
			{
				string buffer(str, str + i);
				buffer += to_string(arg);
				buffer += format(str + i + 1, args...);
				return move(buffer);
			}
		}

		return str;
	}

	//Format at least one argument - move
	template<typename arg_t, typename ... args_t>
	static String format(
		String&& str,
		arg_t&& arg,
		args_t&& ... args
		)
	{
		using namespace std;

		for (size_t i = 0; i < str.size(); i++)
		{
			if (str[i] == '%')
			{
				string buffer(str.substr(0, i));
				buffer += to_string(arg);
				buffer += format(str.c_str() + i + 1, args...);
				return move(buffer);
			}
		}

		return str;
	}

	//Format at least one argument - copy
	template<typename arg_t, typename ... args_t>
	inline String format(
		const String& str,
		arg_t&& arg,
		args_t&& ... args
		)
	{
		return format(str, arg, args ...);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//String helpers
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static std::vector<String>& split(const String &s, char delim, std::vector<String> &elems)
	{
		std::stringstream ss(s);
		String item;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item);
		}

		return elems;
	}

	static std::vector<String> split(const String &s, char delim)
	{
		std::vector<String> elems;
		split(s, delim, elems);

		return elems;
	}
	
	static std::vector<String>& split(const String &str, const String& delim)
	{
		String s(str);
		size_t pos = 0;
		String token;
		std::vector<String> tokens;
		
		while ((pos = s.find(delim)) != String::npos)
		{
			token = s.substr(0, pos);
			tokens.push_back(token);
			s.erase(0, pos + delim.length());
		}
		tokens.push_back(s);
		
		return tokens;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static String trim(const String& str)
	{
		using namespace std;

		if (str == "")
			return "";

		size_t first = max(str.find_first_not_of(' '), str.find_first_not_of('\t'));
		size_t last = min(str.find_last_not_of(' '), str.find_last_not_of('\t'));

		if (first == String::npos)
			first = 0;

		return str.substr(first, (last - first + 1));
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void toLower(String& str)
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			str[i] = ::tolower(str[i]);
		}
	}

	static void toUpper(String& str)
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			str[i] = ::toupper(str[i]);
		}
	}

	static void toLower(char* str)
	{
		size_t sz = strlen(str);
		for (size_t i = 0; i < sz; i++)
		{
			str[i] = ::tolower(str[i]);
		}
	}

	static void toUpper(char* str)
	{
		size_t sz = strlen(str);
		for (size_t i = 0; i < sz; i++)
		{
			str[i] = ::toupper(str[i]);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Compares two strings in a non case sensitive way
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static bool compare_string_weak(const char* str0, const char* str1)
	{
		size_t sz0 = strlen(str0);
		size_t sz1 = strlen(str1);
		if (sz0 != sz1) return false;

		for (size_t i = 0; i < sz0; ++i)
		{
			if (toupper(str0[i]) != toupper(str1[i]))
				return false;
		}

		return true;
	}

	static bool compare_string_weak(const String& str0, const String& str1)
	{
		if (str0.size() != str1.size()) return false;

		for (String::size_type i = 0; i < str0.size(); ++i)
		{
			if (::toupper(str0[i]) != ::toupper(str1[i]))
				return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Fixed size string class - exists on the stack
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<size_t n>
	class StaticString
	{
	private:

		static_assert(n > 0, "number of chars must be greater than zero");
		char m_chars[n];

	public:

		static const size_t npos = n;

		//Constructors

		StaticString()
		{
			memset(m_chars, 0, n);
		}

		StaticString(const char* str) :
			StaticString()
		{
			set(str);
		}

		StaticString(const String& str) :
			StaticString(str.c_str())
		{}

		//Operator overloads

		inline StaticString& operator=(const StaticString<n>& str)
		{
			set(str.m_chars);
			return *this;
		}

		inline bool operator==(const StaticString<n>& str) const
		{
			return compare(str);
		}

		inline bool operator!=(const StaticString<n>& str) const
		{
			return !this->operator==(str);
		}

		inline bool operator<(const StaticString<n>& str) const
		{
			return strcmp(m_chars, str.m_chars) < 0;
		}

		inline bool operator>(const StaticString<n>& str) const
		{
			return strcmp(m_chars, str.m_chars) > 0;
		}

		//methods

		inline bool compare(const StaticString<n>& str) const
		{
			return strcmp(str.m_chars, this->m_chars) == 0;
		}

		inline void set(const String& str, size_t offset = 0)
		{
			set(str.c_str(), offset);
		}

		inline void set(const char* str, size_t offset = 0)
		{
			using namespace std;
			size_t len = strlen(str);
			strncpy_s(m_chars + offset, n - offset, str, min(len, n - offset));
		}

		inline const char* str() const
		{
			return m_chars;
		}

		char* str()
		{
			return m_chars;
		}

		inline char at(size_t i) const
		{
			return m_chars[i];
		}

		inline char operator[](size_t i) const
		{
			return at(i);
		}

		inline char& at(size_t i)
		{
			return m_chars[i];
		}

		inline char& operator[](size_t i)
		{
			return at(i);
		}

		inline size_t length() const
		{
			return strlen(m_chars);
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
}