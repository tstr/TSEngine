/*
	Camera class
*/

#include "camera.h"

using namespace std;
using namespace ts;

enum EActionFlags
{
	eForward = 0x01,
	eBack	 = 0x02,
	eLeft	 = 0x04,
	eRight	 = 0x08,
	eUp		 = 0x10,
	eDown	 = 0x20,
};

///////////////////////////////////////////////////////////////////////////////////////////////
//Input handlers
///////////////////////////////////////////////////////////////////////////////////////////////

int CCamera::onMouse(int16 dx, int16 dy)
{
	m_mouseDX += dx;
	m_mouseDY += dy;

	return 0;
}

int CCamera::onKeyDown(EKeyCode code)
{
	switch (code)
	{
		case eKeyW: m_actionflags |= eForward; break;
		case eKeyS: m_actionflags |= eBack;	   break;
		case eKeyA: m_actionflags |= eLeft;	   break;
		case eKeyD: m_actionflags |= eRight;   break;
		case eKeySpace: m_actionflags |= eUp; break;
		case eKeyCtrlL: m_actionflags |= eDown; break;
	}

	return 0;
}

int CCamera::onKeyUp(EKeyCode code)
{
	switch (code)
	{
		case eKeyW: m_actionflags &= ~eForward; break;
		case eKeyS: m_actionflags &= ~eBack;	break;
		case eKeyA: m_actionflags &= ~eLeft;	break;
		case eKeyD: m_actionflags &= ~eRight;	break;
		case eKeySpace: m_actionflags &= ~eUp;  break;
		case eKeyCtrlL: m_actionflags &= ~eDown; break;
	}

	return 0;

}

int CCamera::onMouseDown(const SInputMouseEvent& args)
{
	if (args.buttons == eMouseButtonRight)
	{
		m_moveMouse = true;
		m_inputmodule->showCursor(false);
	}
	return 0;
}

int CCamera::onMouseUp(const SInputMouseEvent& args)
{
	if (args.buttons == eMouseButtonRight)
	{
		m_moveMouse = false;
		m_inputmodule->showCursor(true);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void CCamera::update(double dt)
{
	//Update position
	int8 actions = m_actionflags.load();
	const float speed = 30.0f;
	float dis = speed * (float)dt;

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