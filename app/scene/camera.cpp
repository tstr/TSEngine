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

///////////////////////////////////////////////////////////////////////////////////////////////

void CCamera::update(double dt)
{
	//Update position
	int8 actions = m_actionflags.load();
	const float speed = 3.0f;
	float dis = speed * (float)dt;

	float dx = m_mouseDX.exchange(0);
	float dy = m_mouseDY.exchange(0);

	const float sensitivity = 0.00012f;
	const float pi2 = 2 * Pi;
	m_camAngleX = m_camAngleX + pi2 * sensitivity * (float)dx;
	m_camAngleY = m_camAngleY + pi2 * sensitivity * (float)dy;
	m_camAngleX = fmod(m_camAngleX, pi2);
	m_camAngleY = fmod(m_camAngleY, pi2);

	float& ax = m_camAngleX;
	float& ay = m_camAngleY;

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