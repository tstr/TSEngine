
#include <string>
#include <tsengine.h>

namespace ts
{
	class Application : public ApplicationCore
	{
	private:

		std::string m_args;

	public:

		Application(const char* cmdargs) : ApplicationCore(cmdargs)
		{

		}

		void onInit() override;

		void onShutdown() override {}
		void onUpdate() override {}
		void onRender() override {}

	};
}