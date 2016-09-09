/*
	Application
*/

#pragma once

#include <string>
#include <tsengine.h>
#include <tsgraphics/rendermodule.h>
#include <tsengine/input/inputmodule.h>
#include <tscore/maths.h>

#include "scene/camera.h"
#include "scene/modelimporter.h"

namespace ts
{
	class CCamera;

	class Application :
		public IApplication,
		public CWindow::IEventListener,
		public IInputEventListener
	{
	private:
		
		CEngineSystem* m_system = nullptr;
		
		UniquePtr<CCamera> m_camera;

		IRenderContext* m_context = nullptr;

		CModelImporter m_model;

		struct SUniforms
		{
			Matrix u_world;
			Matrix u_view;
			Matrix u_projection;
			Vector u_lightdirection;
			Vector u_eyeposition;
		};
		SUniforms m_uniforms;

		CTexture2D m_tex2D;
		CShader m_vertexshader;
		CShader m_pixelshader;
		CUniformBuffer m_uniformBuffer;
		CVertexBuffer m_vertexBuffer;
		CIndexBuffer m_indexBuffer;
		
		ResourceProxy m_texSampler;
		ResourceProxy m_depthTarget;
		ResourceProxy m_vertexInput;
		
		int onWindowEvent(const SWindowEventArgs& args) override;
		int onKeyDown(EKeyCode code) override;

	public:

		Application() {}
		~Application() {}
		
		void onInit(CEngineSystem* system) override;
		void onExit() override;
		void onUpdate(double deltatime) override;		
	};
}
