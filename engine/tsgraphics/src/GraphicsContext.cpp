/*
	Graphics Context source
*/

#include <tsgraphics/GraphicsContext.h>
#include <tsgraphics/api/RenderApi.h>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GraphicsContext::createDraw(const CDrawBuilder& build, HDrawCmd& cmd)
{
	auto api = m_system->getApi();

	SDrawCommand cmdDesc;
	build.get(cmdDesc);

	ERenderStatus status = api->createDrawCommand(cmd, cmdDesc);

	if (status)
	{
		return status;
	}

	m_drawPool.push_back(cmd);

	return 0;
}

int GraphicsContext::destroyDraw(HDrawCmd cmd)
{
	auto it = find(m_drawPool.begin(), m_drawPool.end(), cmd);

	if (it == m_drawPool.end())
	{
		return ERenderStatus::eFail;
	}

	auto api = m_system->getApi();
	api->destroyDrawCommand(cmd);
	
	return 0;
}

void GraphicsContext::clearDraws()
{
	auto api = m_system->getApi();

	for (HDrawCmd cmd : m_drawPool)
	{
		api->destroyDrawCommand(cmd);
	}

	m_drawPool.clear();
}

void GraphicsContext::update()
{
	m_system->execute(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
