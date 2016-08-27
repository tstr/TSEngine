/*
	Application
*/

#include <string>
#include <tsengine.h>
#include <tsgraphics/rendermodule.h>

namespace ts
{
	class Application : public IApplication
	{
	private:

		CEngineSystem* m_system = nullptr;

		SRenderCommand m_command;
		IRenderContext* m_context;

		CTexture2D m_tex2D;
		CShader m_vertexshader;
		CShader m_pixelshader;
		CUniformBuffer m_uniforms;

	public:

		Application() {}
		~Application() {}

		void onInit(CEngineSystem* system) override;
		void onExit() override;
		void onUpdate(double deltatime) override;
	};
}