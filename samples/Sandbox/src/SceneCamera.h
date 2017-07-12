/*
	Camera class
*/

#pragma once

#include <tscore/maths.h>
#include <tsengine/Input.h>
#include <atomic>

namespace ts
{
	class SceneCamera : private InputSystem::IListener
	{
	private:
	
		InputSystem* m_inputSystem = nullptr;
		
		std::atomic<bool> m_moveMouse;

		//Input state
		std::atomic<int8> m_actionflags = 0;
		std::atomic<int16> m_mouseDX = 0;
		std::atomic<int16> m_mouseDY = 0;

		//Camera state
		Vector m_camPosition;
		float m_camSpeed = 10.0f;
		float m_camAngleH = 0.0f; //Rotation of camera horizontally about Y axis
		float m_camAngleV = 0.0f; //Rotation of camera vertically about the X axis

		//Projection state
		float m_fov = Pi / 2;
		float m_aspectRatio = 1.0f;
		const float m_nearplane = 0.1f;
		const float m_farplane = 1000.0f;
		
		//Input handlers
		void onMouseMove(int dx, int dy) override;
		void onKeyDown(EKeyCode code) override;
		void onKeyUp(EKeyCode code) override;
		
	public:
		
		SceneCamera() = default;
		SceneCamera(InputSystem* input);
		~SceneCamera();
		
		void setPosition(Vector v) { m_camPosition = v; }
		Vector getPosition() const { return m_camPosition; }

		void setAspectRatio(float ratio) { m_aspectRatio = ratio; }
		float getAspectRatio() const { return m_aspectRatio; }

		//In radians
		void setFov(float fov) { m_fov = fov; }
		float getFov() const { return m_fov; }

		void setSpeed(float speed) { m_camSpeed = speed; }
		float getSpeed() const { return m_camSpeed; }

		void update(double deltatime);
		
		Matrix getViewMatrix() const { return (Matrix::rotationX(m_camAngleV) * Matrix::rotationY(m_camAngleH) * Matrix::translation(m_camPosition)).inverse(); }
		Matrix getProjectionMatrix() const { return Matrix::perspectiveFieldOfView(m_fov, m_aspectRatio, m_nearplane, m_farplane); }
	};
	
}

