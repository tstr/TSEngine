/*
	Sandbox application
*/

#include <tsengine/env.h>
#include <tsengine/input/inputmodule.h>

#include "graphics/Graphics3D.h"
#include "Camera.h"

namespace ts
{
	class Sandbox : public IApplication
	{
	private:
		
		CEngineEnv& mEnv;
		Graphics3D m_g3D;

		CCamera m_camera;

		std::vector<CRenderItem> m_commands;

		int onInit() override;
		void onExit() override;
		void onUpdate(double deltatime) override;

	public:

		Sandbox(CEngineEnv& env);
		~Sandbox();
	};
}
