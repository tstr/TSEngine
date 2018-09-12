/*
	Shadow Mapping pipeline
*/

#include "ShadowPass.h"

#include <tscore/debug/assert.h>

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

ShadowPass::ShadowPass(RenderDevice* device, uint32 width, uint32 height)
{
	tsassert(device);

	/*
		Create render targets
	*/

	m_shadowBuffer = ImageTarget::createColourTarget(
		device,
		width, height,
		ImageFormat::FLOAT2,
		1,
		true
	);

	m_shadowDepth = ImageTarget::createDepthTarget(
		device,
		width, height,
		ImageFormat::DEPTH32
	);

	Viewport viewp;
	viewp.w = width;
	viewp.h = height;

	m_shadowTarget = RenderTargets<>(device);
	m_shadowTarget.attach(0, m_shadowBuffer.getView());
	m_shadowTarget.attachDepth(m_shadowDepth.getView());
	m_shadowTarget.setViewport(viewp);
	m_shadowTarget.refresh();
}

///////////////////////////////////////////////////////////////////////////////
