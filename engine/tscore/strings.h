/*
	String header - contains helpers for strings
*/

#pragma once

#include "types.h"
#include <sstream>
#include <vector>

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
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Format string - allows arguments to be inserted to locations in the string marked by the '%' char
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Format no arguments
	inline std::string format(const std::string& str)
	{
		return str;
	}

	//Format at least one argument - cstring
	template<typename arg_t, typename ... args_t>
	static std::string format(
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
	static std::string format(
		std::string&& str,
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
	inline std::string format(
		const std::string& str,
		arg_t&& arg,
		args_t&& ... args
		)
	{
		return format(str, arg, args ...);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//String splitters

	static std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems)
	{
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item);
		}

		return elems;
	}

	static std::vector<std::string> split(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		split(s, delim, elems);

		return elems;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Compares two strings in a non case sensitive way
	static bool compare_string_weak(const std::string& str0, const std::string& str1)
	{
		if (str0.size() != str1.size()) return false;

		for (std::string::size_type i = 0; i < str0.size(); ++i)
		{
			if (toupper(str0[i]) != toupper(str1[i]))
				return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
}