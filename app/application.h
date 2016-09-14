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
		UniquePtr<CModel> m_sphere;

		IRenderContext* m_context = nullptr;

		float m_pulsatance = 0.0f;
		atomic<bool> m_simulation = true;
		atomic<bool> m_mouseHeld = false;

		CShader m_standardVertexshader;
		CShader m_standardPixelshader;
		CShader m_lightVertexShader;
		CShader m_lightPixelShader;
		CShader m_shadowVertexShader;
		CShader m_shadowPixelShader;

		CUniformBuffer m_sceneBuffer;
		CUniformBuffer m_materialBuffer;
		CUniformBuffer m_shadowSceneBuffer;
		
		ResourceProxy m_texSampler;
		ResourceProxy m_depthTarget;
		ResourceProxy m_vertexInput;
		ResourceProxy m_vertexInputLight;
		ResourceProxy m_vertexInputShadow;
		
		ResourceProxy m_shadowDepthTarget;
		ResourceProxy m_shadowCubeRsc;
		ResourceProxy m_shadowCube;
		
		int onWindowEvent(const SWindowEventArgs& args) override;
		int onKeyDown(EKeyCode code) override;
		int onMouseDown(const SInputMouseEvent&) override;
		int onMouseUp(const SInputMouseEvent&) override;
		int onMouse(int16 dx, int16 dy) override;

		void buildDepthTarget();

	public:

		Application();
		~Application();
		
		void onInit(CEngineSystem* system) override;
		void onExit() override;
		void onUpdate(double deltatime) override;		
	};
}
