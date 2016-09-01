/*
	Application
*/

#include <string>
#include <tsengine.h>
#include <tsgraphics/rendermodule.h>
#include <tsengine/input/inputmodule.h>

namespace ts
{
	class Application : public IApplication
	{
	private:

		class InputListener : public IInputEventListener
		{
		private:

			Application* m_pApp = nullptr;

		public:

			InputListener(Application* app) :
				m_pApp(app)
			{}

			int onMouse(int16 dx, int16 dy) override;
			int onKeyDown(EKeyCode code) override;
		};

		CEngineSystem* m_system = nullptr;
		InputListener m_inputListener;

		SRenderCommand m_command;
		IRenderContext* m_context;

		CTexture2D m_tex2D;
		CShader m_vertexshader;
		CShader m_pixelshader;
		CUniformBuffer m_uniforms;

	public:

		Application() : m_inputListener(this) {}
		~Application() {}

		void onInit(CEngineSystem* system) override;
		void onExit() override;
		void onUpdate(double deltatime) override;
	};
}