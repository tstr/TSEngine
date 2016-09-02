/*
	Application
*/

#include <string>
#include <tsengine.h>
#include <tsgraphics/rendermodule.h>
#include <tsengine/input/inputmodule.h>
#include <tscore/maths.h>

namespace ts
{
	class Application :
		public IApplication,
		public CWindow::IEventListener
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
			int onKeyUp(EKeyCode code) override;
		};

		CEngineSystem* m_system = nullptr;
		InputListener m_inputListener;

		SRenderCommand m_command;
		IRenderContext* m_context = nullptr;

		struct Uniforms
		{
			Matrix u_world;
			Matrix u_view;
			Matrix u_projection;
			Vector u_direction;
		};
		Uniforms m_uniforms;

		CTexture2D m_tex2D;
		CShader m_vertexshader;
		CShader m_pixelshader;
		CUniformBuffer m_uniformBuffer;
		CVertexBuffer m_vertexBuffer;
		CIndexBuffer m_indexBuffer;

		atomic<int8> m_actionflags = 0;
		Vector m_camPosition;
		atomic<float> m_camAngleX = 0.0f;
		atomic<float> m_camAngleY = 0.0f;

		int onWindowEvent(const SWindowEventArgs& args) override;

	public:

		Application() : m_inputListener(this) {}
		~Application() {}

		void onInit(CEngineSystem* system) override;
		void onExit() override;
		void onUpdate(double deltatime) override;
	};
}
