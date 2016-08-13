/*
	Logging api
*/

#pragma once

#include <tscore/strings.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum ELogLevel
	{
		eLevelInfo = 0,
		eLevelWarn = 1,
		eLevelError = 2,
		eLevelProfile = 3
	};

	class ILogStream
	{
	public:

		virtual void write(const char* message, ELogLevel level) = 0;
	};

	class CDefaultLogStream : public ILogStream
	{
		void write(const char* message, ELogLevel level) override;
	};
	
	class CLog
	{
	public:

		CLog(ILogStream* s = nullptr) :
			m_stream(s)
		{}

		void operator()(
			const char* message,
			ELogLevel level,
			char const* function,
			char const* file,
			int line
		);

		void setStream(ILogStream* s)
		{
			m_stream = s;
		}

	private:

		ELogLevel m_level = ELogLevel::eLevelInfo;
		ILogStream* m_stream;
	};

	namespace global
	{
		inline CLog& getLogger()
		{
			static CDefaultLogStream s_defaultStream;
			static CLog s_log(&s_defaultStream);
			return s_log;
		}
	}

#define _tslogwrite(logger, message, level, ...) \
	logger(                                      \
	ts::format(message, __VA_ARGS__).c_str(),	 \
	level,										 \
	__FUNCTION__,                                \
	__FILE__,                                    \
	__LINE__                                     \
  )

#define tsinfo(message, ...) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelInfo, __VA_ARGS__)
#define tswarn(message, ...)  _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelWarn, __VA_ARGS__)
#define tserror(message, ...) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelError, __VA_ARGS__)
#define tsprofile(message, ...) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelProfile, __VA_ARGS__)
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////