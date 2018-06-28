/*
	Camera class
*/

#include "Camera.h"

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////

Camera::Camera(InputSystem* input) :
	m_inputSystem(input)
{
	m_inputSystem->addListener(this);
	m_moveMouse = false;
}

Camera::~Camera()
{
	m_inputSystem->removeListener(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Input handlers
///////////////////////////////////////////////////////////////////////////////////////////////

enum EActionFlags
{
	eForward = 0x01,
	eBack	 = 0x02,
	eLeft	 = 0x04,
	eRight	 = 0x08,
	eUp		 = 0x10,
	eDown	 = 0x20,
	eSpeed   = 0x40,
};

void Camera::onMouseMove(int dx, int dy)
{
	m_mouseDX += dx;
	m_mouseDY += dy;
}

void Camera::onKeyDown(EKeyCode code)
{
	switch (code)
	{
		case eKeyW: m_actionflags |= eForward; break;
		case eKeyS: m_actionflags |= eBack;	   break;
		case eKeyA: m_actionflags |= eLeft;	   break;
		case eKeyD: m_actionflags |= eRight;   break;
		case eKeySpace: m_actionflags |= eUp; break;
		case eKeyCtrlL: m_actionflags |= eDown; break;
		case eKeyShiftL:m_actionflags |= eSpeed; break;
	}

	if (code == eMouseButtonRight)
	{
		m_moveMouse = true;
		m_inputSystem->showCursor(false);
	}
}

void Camera::onKeyUp(EKeyCode code)
{
	switch (code)
	{
		case eKeyW: m_actionflags &= ~eForward; break;
		case eKeyS: m_actionflags &= ~eBack;	break;
		case eKeyA: m_actionflags &= ~eLeft;	break;
		case eKeyD: m_actionflags &= ~eRight;	break;
		case eKeySpace: m_actionflags &= ~eUp;  break;
		case eKeyCtrlL: m_actionflags &= ~eDown; break;
		case eKeyShiftL:m_actionflags &= ~eSpeed; break;
	}

	if (code == eMouseButtonRight)
	{
		m_moveMouse = false;
		m_inputSystem->showCursor(true);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

void Camera::update(double dt)
{
	//Update position
	int8 actions = m_actionflags.load();

	float speed = m_camSpeed;

	if (actions & eSpeed)
	{
		speed = speed * 3.0f;
	}

	//Distance travelled over dt seconds
	const float dis = speed * (float)dt;

	//Reset the value of the x/y mouse displacement
	float dx = (float)m_mouseDX.exchange(0);
	float dy = (float)m_mouseDY.exchange(0);

	if (m_moveMouse.load())
	{
		const float sensitivity = 0.00012f;
		const float pi2 = 2 * Pi;
		m_camAngleH = m_camAngleH + pi2 * sensitivity * (float)dx;
		m_camAngleV = m_camAngleV + pi2 * sensitivity * (float)dy;
		m_camAngleH = fmod(m_camAngleH, pi2);
		m_camAngleV = fmod(m_camAngleV, pi2);
	}

	//Clamp the vertical rotation of the camera between -90 and 90 degrees
	m_camAngleV = max(min(m_camAngleV, Pi / 2), -(Pi / 2));

	float& ax = m_camAngleH;//horizontal
	float& ay = m_camAngleV;//vertical

	if (actions & eForward)
	{
		m_camPosition.x() += dis * sin(ax);
		m_camPosition.z() += dis * cos(ax);
	}
	if (actions & eBack)
	{
		m_camPosition.x() -= dis * sin(ax);
		m_camPosition.z() -= dis * cos(ax);
	}
	if (actions & eLeft)
	{
		m_camPosition.x() -= dis * cos(ax);
		m_camPosition.z() += dis * sin(ax);
	}
	if (actions & eRight)
	{
		m_camPosition.x() += dis * cos(ax);
		m_camPosition.z() -= dis * sin(ax);
	}
	if (actions & eUp)
	{
		m_camPosition.y() += dis;
	}
	if (actions & eDown)
	{
		m_camPosition.y() -= dis;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
