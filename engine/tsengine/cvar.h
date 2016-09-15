/*
	CVar system header
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
	enum ECVarTypes
	{
		eCVarTypeUnknown = 0,
		eCVarTypeNumber  = 1,
		eCVarTypeVector  = 2,
		eCVarTypeString	 = 3,
	};
	
	enum
	{
		MaxCVarLength = 128,
	};
	
	class CVarTable
	{
	public:

		typedef std::string CVarString;
		
		struct SCVarPair
		{
			CVarString name;
			CVarString value;
		};

		typedef std::vector<SCVarPair> CVarArray;
		
		bool getVarFloat(CVarString name, float& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			auto str = m_cvars.find(name)->second;
			val = stof(str);
			return true;
		}

		bool getVarInt(CVarString name, int& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			auto str = m_cvars.find(name)->second;
			val = stoi(str);
			return true;
		}

		bool getVarBool(CVarString name, bool& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			auto str = m_cvars.find(name)->second;
			val = (stoi(str) != 0);
			return true;
		}
		
		bool getVarVector4D(CVarString name, Vector& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			auto tokens = split(m_cvars.find(name)->second, ',');
			if (tokens.size() > 0) val.x() = stof(trim(tokens[0]));
			if (tokens.size() > 1) val.y() = stof(trim(tokens[1]));
			if (tokens.size() > 2) val.z() = stof(trim(tokens[2]));
			if (tokens.size() > 3) val.w() = stof(trim(tokens[3]));
			return true;
		}

		bool getVarVector3D(CVarString name, Vector& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			auto tokens = split(m_cvars.find(name)->second, ',');
			if (tokens.size() > 0) val.x() = stof(trim(tokens[0]));
			if (tokens.size() > 1) val.y() = stof(trim(tokens[1]));
			if (tokens.size() > 2) val.z() = stof(trim(tokens[2]));
			return true;
		}

		bool getVarVector2D(CVarString name, Vector& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			auto tokens = split(m_cvars.find(name)->second, ',');
			if (tokens.size() > 0) val.x() = stof(trim(tokens[0]));
			if (tokens.size() > 1) val.y() = stof(trim(tokens[1]));
			return true;
		}
		
		bool getVarString(CVarString name, CVarString& val) const
		{
			if (!isVar(name))
				return false;
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			val = m_cvars.find(name)->second;
			return true;
		}
		
		template<typename T>
		void setVar(CVarString name, const T& val)
		{
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			m_cvars[name] = std::to_string(val).c_str();
		}
		
		template<>
		void setVar(CVarString name, const Vector& val)
		{
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			std::stringstream ss;
			ss << val.x() << ", ";
			ss << val.y() << ", ";
			ss << val.z() << ", ";
			ss << val.w();
			m_cvars[name] = ss.str().c_str();
		}
		
		template<>
		void setVar(CVarString name, const CVarString& val)
		{
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			m_cvars[name] = val;
		}
		
		bool isVar(CVarString name) const
		{
			std::lock_guard<std::mutex>lk(m_mutex);
			toLower(name);
			return m_cvars.find(name) != m_cvars.end();
		}

		void getVarArray(CVarArray& vec) const
		{
			vec.clear();
			for (auto it = m_cvars.begin(); it != m_cvars.end(); it++)
			{
				SCVarPair pair;
				pair.name = it->first;
				pair.value = it->second;
				vec.push_back(pair);
			}
		}
		
	private:
		
		mutable std::mutex m_mutex;
		std::unordered_map<CVarString, CVarString> m_cvars;
	};
}
