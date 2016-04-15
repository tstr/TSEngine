/*
	Configuration handling class
*/

#pragma once

#include <C3E\core\corecommon.h>
#include <C3E\core\memory.h>
#include <C3E\core\strings.h>

namespace C3E
{
	class Config
	{
	private:

		struct Impl;
		std::unique_ptr<Impl> pImpl;

	public:

		Config();
		Config(const char*);
		~Config();

		bool Open(const char* cfg);
		const char* FindEntry(const char* path, const char* def = "");
		void Close();
	};

	class CommandLine
	{
	public:

		class Argument
		{
		private:

			std::string m_tag;
			std::string m_value;

		public:

			Argument() {}
			Argument(const char* _tag, const char* _value) :
				m_tag(_tag), m_value(_value)
			{}

			Argument& operator=(const Argument& from)
			{
				m_tag = from.m_tag;
				m_value = from.m_value;
				return *this;
			}

			bool Exists() { return (!m_tag.empty()); }

			const char* GetTag() const { return m_tag.c_str(); }
			const char* GetValue() const { return m_value.c_str(); }

			bool empty() { return (m_tag.empty() && m_value.empty()); }
		};

		CommandLine(const char* cmd_line);

		const char* GetRawCommandLine() const { return m_buffer.c_str(); }

		bool ArgumentTagExists(const char* tag);

		Argument GetArgument(int index)
		{
			return ParseArgument(m_command_table[index].c_str());
		}

		Argument GetArgument(const char* tag);

	private:

		std::string m_buffer;
		std::vector<std::string> m_command_table;

		Argument ParseArgument(const char* str);
	};
}