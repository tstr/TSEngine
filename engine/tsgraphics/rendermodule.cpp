/*
	Graphics module source
*/

#include "rendermodule.h"
#include <tscore/debug/log.h>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

ResourceProxy textureRsc;
ResourceProxy textureView;

CRenderModule::CRenderModule(const SRenderModuleConfiguration& cfg) :
	m_config(cfg)
{
	if (!loadApi(cfg.apiEnum))
		tserror("Unable to load graphics api (ERenderApiID::eRenderApiD3D11)");

	STextureResourceData data;
	Vector vec(1, 0.4f, 0.2f, 1);
	data.memory = (const void*)&vec;
	data.memoryByteWidth = sizeof(Vector);
	data.memoryByteDepth = 0;
	STextureResourceDescriptor desc;
	desc.textype = ETextureResourceType::eTypeTexture2D;
	desc.texmask = eTextureMaskShaderResource | eTextureMaskRenderTarget;
	desc.height = 1;
	desc.width = 1;
	desc.multisampling.count = 1;
	desc.texformat = ETextureFormat::eTextureFormatFloat3;
	desc.depth = 0;
	desc.useMips = false;
	ERenderStatus status = m_api->createResourceTexture(textureRsc, &data, desc);

	if (status) { tserror("%", status); }

	STextureViewDescriptor viewdesc;
	viewdesc.arrayIndex = 0;
	viewdesc.arrayCount = 1;
	status = m_api->createTargetRender(textureView, textureRsc, viewdesc);

	if (status) { tserror("%", status); }
}

CRenderModule::~CRenderModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::setWindowMode(EWindowMode mode)
{
	m_api->setWindowMode(mode);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::drawBegin(const Vector& vec)
{
	m_api->drawBegin(vec);
	//m_api->test(textureView);
}

void CRenderModule::drawEnd()
{
	m_api->drawEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////////////