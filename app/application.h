
#include <string>
#include <tsengine.h>

namespace ts
{
	class Application : public IApplication
	{
	public:

		Application() {}
		~Application();

		void onInit() override;
		void onExit() override;
		void onUpdate() override;
		void onRender() override;

	};
}