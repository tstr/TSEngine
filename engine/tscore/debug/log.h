/*
	Logging api
*/

#include <ostream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum ELogLevel
	{
		eLevelDebug = 0,
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

		ELogLevel m_level = ELogLevel::eLevelDebug;
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

#define _tslogwrite(logger, message, level)	 	 \
	logger(                                      \
	static_cast<std::ostringstream&>(            \
		std::ostringstream().flush() << (message)\
	).str().c_str(),                             \
	level,										 \
	__FUNCTION__,                                \
	__FILE__,                                    \
	__LINE__                                     \
  )

#define tsprint(message) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelDebug)
#define tswarn(message)	 _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelWarn)
#define tserror(message) _tslogwrite(::ts::global::getLogger(), message, ::ts::eLevelError)

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////