/*
	Logging api
*/

#pragma once

#include <tscore/abi.h>
#include <tscore/strings.h>
#include <ctime>

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

	//A log message
	struct SLogMessage
	{
		typedef time_t TimeStamp;

		StaticString<2048> message;
		StaticString<256> file;
		StaticString<256> function;
		size_t line = 0;
		ELogLevel level;
		TimeStamp timestamp;

		SLogMessage() {}

		SLogMessage(
			const char* _msg,
			const char* _file,
			const char* _func,
			size_t _line,
			ELogLevel _level
		) :
			message(_msg), file(_file), function(_func), line(_line), level(_level),
			timestamp(::time(0))
		{}
	};

	class ILogStream
	{
	public:

		virtual void write(const SLogMessage& msg) = 0;
	};

	class CLog
	{
	public:

		CLog() {}
		~CLog() {}

		void TSCORE_API operator()(
			const SLogMessage& msg
		);

		void addStream(ILogStream* stream)
		{
			m_streams.push_back(stream);
		}

		void detachStream(ILogStream* stream)
		{
			auto it = find(m_streams.begin(), m_streams.end(), stream);
			
			if (it != m_streams.end())
				m_streams.erase(it);
		}

	private:

		std::vector<ILogStream*> m_streams;
	};

	namespace global
	{
		TSCORE_API CLog& getLogger();
	}

#define _tslogwrite(logger, message, level, ...) \
	logger(										  \
	SLogMessage(								  \
		ts::format(message, __VA_ARGS__).c_str(), \
		__FILE__,                                 \
		__FUNCTION__,                             \
		__LINE__,                                 \
		level								  \
	)\
  )

#define tsinfo(message, ...) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelInfo, __VA_ARGS__)
#define tswarn(message, ...)  _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelWarn, __VA_ARGS__)
#define tserror(message, ...) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelError, __VA_ARGS__)
#define tsprofile(message, ...) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelProfile, __VA_ARGS__)
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////