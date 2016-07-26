
#include <string>
#include <tsengine.h>

namespace ts
{
	class Application
	{
	private:

		std::string m_args;

	public:

		Application(const char* cmdargs) 
		{
			m_args = cmdargs;
			init();
		}

		void init();
	};
}