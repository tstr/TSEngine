/*
	String header - contains helpers for strings
*/

#pragma once

#include "types.h"
#include <sstream>
#include <vector>

#define tocstr(x) std::to_string((x)).c_str()

namespace ts
{
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

}