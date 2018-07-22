/*
	Shadow Mapping pipeline
*/

#include "ShadowMap.h"

#include <tscore/debug/assert.h>

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

ShadowMap::ShadowMap(GraphicsSystem* gfx, uint32 width, uint32 height)
{
	tsassert(gfx);

	/*
		Create render targets
	*/

	m_shadowBuffer = ImageTarget::createColourTarget(
		gfx->device(),
		width, height,
		ImageFormat::FLOAT2,
		1,
		true
	);

	m_shadowDepth = ImageTarget::createDepthTarget(
		gfx->device(),
		width, height,
		ImageFormat::DEPTH32
	);

	Viewport viewp;
	viewp.w = width;
	viewp.h = height;

	m_shadowTarget = RenderTargets<>(gfx->device());
	m_shadowTarget.attach(0, m_shadowBuffer.getView());
	m_shadowTarget.attachDepth(m_shadowDepth.getView());
	m_shadowTarget.setViewport(viewp);
	m_shadowTarget.refresh();

	/*
		Create shader
	*/

	Path shaderPath(gfx->getRootPath());
	shaderPath.addDirectories("shaderbin/ShadowMap.shader");

	tsassert(m_shadowShader.load(gfx->device(), shaderPath.str()));
}

///////////////////////////////////////////////////////////////////////////////
