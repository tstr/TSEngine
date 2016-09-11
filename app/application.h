/*
	Application
*/

#pragma once

#include <string>
#include <tsengine.h>
#include <tsgraphics/rendermodule.h>
#include <tsengine/input/inputmodule.h>
#include <tscore/maths.h>

namespace ts
{
	class CCamera;
	class CModel;

	class Application :
		public IApplication,
		public CWindow::IEventListener,
		public IInputEventListener
	{
	private:
		
		CEngineSystem* m_system = nullptr;
		
		UniquePtr<CCamera> m_camera;
		UniquePtr<CModel> m_model;

		IRenderContext* m_context = nullptr;

		struct SUniforms
		{
			Matrix u_world;
			Matrix u_view;
			Matrix u_projection;
			Vector u_lightdirection;
			Vector u_eyeposition;
		};
		SUniforms m_uniforms;

		CShader m_vertexshader;
		CShader m_pixelshader;
		CUniformBuffer m_uniformBuffer;
		
		ResourceProxy m_texSampler;
		ResourceProxy m_depthTarget;
		ResourceProxy m_vertexInput;
		
		int onWindowEvent(const SWindowEventArgs& args) override;
		int onKeyDown(EKeyCode code) override;
		int onMouseDown(const SInputMouseEvent&) override;
		int onMouseUp(const SInputMouseEvent&) override;

		void buildDepthTarget();

	public:

		Application();
		~Application();
		
		void onInit(CEngineSystem* system) override;
		void onExit() override;
		void onUpdate(double deltatime) override;		
	};
}
