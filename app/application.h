
#include <string>
#include <tsengine.h>

namespace ts
{
	class Application : public IApplication
	{
	private:

		std::string m_args;

	public:

		Application() {}

		void onInit() override;
		void onDeinit() override {}
		void onUpdate() override {}
		void onRender() override {}

	};
}