/*
	Render Item class implementation
*/

#include <tsgraphics/GraphicsContext.h>
#include <tsgraphics/api/RenderApi.h>

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRenderItem::CRenderItem(GraphicsContext* context, const CRenderItemInfo& info) :
	m_context(context),
	m_command(HDRAWCMD_NULL),
	m_commandInfo(info)
{
	SDrawCommand cmdDesc;
	m_commandInfo.get(cmdDesc);
	
	if (m_context->allocDraw(cmdDesc, m_command))
	{
		m_command = HDRAWCMD_NULL;
		m_context = nullptr;
		m_commandInfo.reset();
	}
}

CRenderItem::CRenderItem(CRenderItem&& rhs) :
	m_context(nullptr),
	m_command(HDRAWCMD_NULL)
{
	swap(m_command, rhs.m_command);
	swap(m_context, rhs.m_context);
	swap(m_commandInfo, rhs.m_commandInfo);
}

CRenderItem& CRenderItem::operator=(CRenderItem&& rhs)
{
	swap(m_command, rhs.m_command);
	swap(m_context, rhs.m_context);
	swap(m_commandInfo, rhs.m_commandInfo);

	return *this;
}

CRenderItem::~CRenderItem()
{
	clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderItem::clear()
{
	if (m_context && m_command)
	{
		m_context->getSystem()->getApi()->destroyDrawCommand(m_command);
		m_command = HDRAWCMD_NULL;
	}
}

void CRenderItem::setInfo(const CRenderItemInfo& info)
{
	clear();

	m_commandInfo = info;

	SDrawCommand cmdDesc;
	m_commandInfo.get(cmdDesc);

	if (m_context->allocDraw(cmdDesc, m_command))
	{
		m_command = HDRAWCMD_NULL;
		m_context = nullptr;
		m_commandInfo.reset();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
