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
			Matrix world;
			Matrix invWorld;
			Matrix view;
			Matrix invView;
			Matrix projection;
			Matrix invProjection;

			Vector lightPos;
			Vector lightColour;
			Vector globalAmbientColour;
			Vector eyePos;

			float nearplane;
			float farplane;

			float lightConstantAttenuation;
			float lightLinearAttenuation;
			float lightQuadraticAttenuation;

			void init()
			{
				invWorld = world.inverse();
				invView = view.inverse();
				invProjection = projection.inverse();

				Matrix::transpose(invWorld);
				Matrix::transpose(invView);
				Matrix::transpose(invProjection);
				Matrix::transpose(world);
				Matrix::transpose(view);
				Matrix::transpose(projection);
			}
		};
		SUniforms m_uniforms;

		float m_pulsatance = 0.0f;

		CShader m_vertexshader;
		CShader m_pixelshader;
		CUniformBuffer m_sceneBuffer;
		CUniformBuffer m_materialBuffer;
		
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
