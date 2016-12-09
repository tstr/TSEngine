/*
	Application
*/

#pragma once

#include <string>
#include <tsengine.h>
#include <tscore/maths.h>
#include <tsgraphics/rendermodule.h>
#include <tsengine/input/inputmodule.h>

namespace ts
{
	class CCamera;
	class CModel;
	class UISystem;
	class UICommandConsole;
	class UIDebugMenu;
	
	class Application :
		public IApplication,
		public CWindow::IEventListener,
		public IInputEventListener
	{
	private:
		
		CEngineEnv& m_env;
		
		UniquePtr<CCamera> m_camera;
		UniquePtr<CModel> m_model;
		UniquePtr<CModel> m_sphere;
		UniquePtr<UISystem> m_ui;
		UniquePtr<UICommandConsole> m_consoleMenu;
		UniquePtr<UIDebugMenu> m_debugMenu;

		IRenderContext* m_context = nullptr;

		float m_pulsatance = 0.0f;
		bool m_showConsole = false;
		bool m_showUI = true;
		bool m_simulation = true;
		bool m_mouseHeld = false;
		float m_scrollDepth = 0.0f;

		std::deque<float> m_frametimes;
		std::deque<float> m_framerates;
		uint64 m_frameno = 0;
		double m_frametime = 0.0;
		
		CTextureCube m_skybox;

		ShaderId m_standardVertexshader = 0;
		ShaderId m_standardPixelshader  = 0;
		ShaderId m_lightVertexShader    = 0;
		ShaderId m_lightPixelShader		= 0;
		ShaderId m_shadowVertexShader	= 0;
		ShaderId m_shadowPixelShader	= 0;
		ShaderId m_skyboxVertexShader	= 0;
		ShaderId m_skyboxPixelShader	= 0;

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
		int onMouseScroll(const SInputMouseEvent&) override;
		int onMouseDown(const SInputMouseEvent&) override;
		int onMouseUp(const SInputMouseEvent&) override;
		int onMouse(int16 dx, int16 dy) override;

		bool m_rebuildDepthTarget = false;
		void buildDepthTarget();
		void buildVertexInputDescriptor(std::vector<SShaderInputDescriptor>& inputdescriptor, uint32 vertexFlags);
		
	public:

		Application(CEngineEnv& system);
		~Application();

		CEngineEnv const&  getSystem() const { return m_env; }
		
		int onInit() override;
		void onExit() override;
		void onUpdate(double deltatime) override;		
	};
}
