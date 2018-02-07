/*
	Render Pass source
*/

#include <tsgraphics/GraphicsContext.h>
#include <tsgraphics/api/RenderApi.h>

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctor/dtor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsContext::Pass::Pass(GraphicsContext* context) :
	m_context(context),
	m_target(HTARGET_NULL),
	m_targetTexture(HTEXTURE_NULL),
	m_targetIsDisplay(false)
{

}

GraphicsContext::Pass::~Pass()
{
	resetItemCommands();
	resetTargets();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Target methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsContext::Pass::initTarget2D(uint width, uint height, uint multisampling)
{
	auto gfx = m_context->getSystem();
	auto api = gfx->getApi();

	STextureResourceDesc txDesc;
	txDesc.arraySize = 1;
	txDesc.depth = 0;
	txDesc.width = width;
	txDesc.height = height;
	txDesc.multisampling = multisampling;
	txDesc.texformat = ETextureFormat::eTextureFormatColourRGBA;
	txDesc.texmask = ETextureResourceMask::eTextureMaskRenderTarget | ETextureResourceMask::eTextureMaskShaderResource;
	txDesc.textype = ETextureResourceType::eTypeTexture2D;
	txDesc.useMips = false;

	// Create target texture
	tsassert(api->createResourceTexture(m_targetTexture, nullptr, txDesc) == ERenderStatus::eOk);

	STargetDesc tDesc;
	tDesc.depthTexture = HTEXTURE_NULL;
	tDesc.depthTextureIndex = 0;
	tDesc.renderTextures[0] = m_targetTexture;
	tDesc.renderTextureIndices[0] = 0;

	// Create target
	tsassert(api->createTarget(m_target, tDesc) == ERenderStatus::eOk);

	m_targetIsDisplay = false;
}

void GraphicsContext::Pass::initDisplayTarget()
{
	m_target = m_context->getSystem()->getDisplayTarget();
	m_targetIsDisplay = true;
}

void GraphicsContext::Pass::resetTargets()
{
	auto gfx = m_context->getSystem();
	auto api = gfx->getApi();

	//Only reset resources if the target is not the display 
	if (!m_targetIsDisplay && (m_target != HTARGET_NULL))
	{
		api->destroyTarget(m_target);

		api->destroyTexture(m_targetTexture);
	}

	m_target = HTARGET_NULL;
	m_targetTexture = HTEXTURE_NULL;

	m_targetIsDisplay = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Render Item methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsContext::Pass::setItemCommand(ItemID item, const SDrawCommand& command)
{
	//Check if Item index is out of range
	if (HandleInfo<ItemID>(item).index >= m_itemInstances.size())
	{
		//Grow instance array
		m_itemInstances.resize(m_itemInstances.size() + 1);
	}

	//Get instance
	ItemInstance& inst = m_itemInstances.at(HandleInfo<ItemID>(item).index);

	//If a HDrawCmd handle has already been created, destroy it
	if (inst.cmd != HDRAWCMD_NULL)
	{
		m_context->getSystem()->getApi()->destroyDrawCommand(inst.cmd);
		inst.cmd = HDRAWCMD_NULL;
	}

	//Create a new render command
	ERenderStatus status = m_context->getSystem()->getApi()->createDrawCommand(inst.cmd, command);
	tsassert(status == ERenderStatus::eOk);
	
	inst.cmdDesc = command;
}

void GraphicsContext::Pass::getItemCommandDesc(ItemID item, SDrawCommand& command) const
{
	const auto idx = HandleInfo<ItemID>(item).index;

	//Check if Item not valid
	if (idx >= m_itemInstances.size() || !m_context->isItem(item))
	{
		return;
	}

	//Get instance from array
	command = m_itemInstances.at(idx).cmdDesc;
}

uint32 GraphicsContext::Pass::getItemCommandHash(ItemID item) const
{
	const auto idx = HandleInfo<ItemID>(item).index;

	//Check if Item not valid
	if (idx >= m_itemInstances.size() || !m_context->isItem(item))
	{
		return 0;
	}

	//Get instance from array
	return m_itemInstances.at(idx).cmdHash;
}

HDrawCmd GraphicsContext::Pass::getItemCommand(ItemID item) const
{
	const auto idx = HandleInfo<ItemID>(item).index;

	//Check if Item not valid
	if ((idx >= m_itemInstances.size()) || !m_context->isItem(item))
	{
		return HDRAWCMD_NULL;
	}

	//Get instance from array
	return  m_itemInstances.at(idx).cmd;
}

void GraphicsContext::Pass::resetItemCommands()
{
	for (ItemInstance& inst : m_itemInstances)
	{
		m_context->getSystem()->getApi()->destroyDrawCommand(inst.cmd);
		inst.cmd = HDRAWCMD_NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
