/*
	Camera class
*/

#pragma once

#include <tscore/maths.h>
#include <tsengine/input/inputmodule.h>
#include <atomic>

namespace ts
{
	class CCamera : public IInputEventListener
	{
	private:
	
		CInputModule* m_inputmodule = nullptr;
		
		std::atomic<bool> m_moveMouse;

		//Input state
		std::atomic<int8> m_actionflags = 0;
		std::atomic<int16> m_mouseDX = 0;
		std::atomic<int16> m_mouseDY = 0;

		//Camera state
		Vector m_camPosition;
		float m_camAngleX = 0.0f;
		float m_camAngleY = 0.0f;
		
		//Projection state
		float m_fov = Pi / 2;
		float m_aspectRatio = 1.0f;
		const float m_nearplane = 0.1f;
		const float m_farplane = 200.0f;
		
		//Input handlers
		int onMouse(int16 dx, int16 dy) override;
		int onKeyDown(EKeyCode code) override;
		int onKeyUp(EKeyCode code) override;
		int onMouseDown(const SInputMouseEvent& args) override;
		int onMouseUp(const SInputMouseEvent& args) override;
		
	public:
		
		CCamera() {}
		
		CCamera(CInputModule* input) :
			m_inputmodule(input)
		{
			m_inputmodule->addEventListener(this);
			m_moveMouse = false;
		}

		~CCamera() {}
		
		void setPosition(Vector v) { m_camPosition = v; }
		void setAspectRatio(float ratio) { m_aspectRatio = ratio; }
		void setFov(float fov) { m_fov = fov; }
		
		void update(double deltatime);
		
		Matrix getViewMatrix() const { return (Matrix::rotationX(m_camAngleY) * Matrix::rotationY(m_camAngleX) * Matrix::translation(m_camPosition)).inverse(); }
		Matrix getProjectionMatrix() const { return Matrix::perspectiveFieldOfView(m_fov, m_aspectRatio, m_nearplane, m_farplane); }
	};
	
}
