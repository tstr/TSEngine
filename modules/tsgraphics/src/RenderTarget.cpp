/*
	Render Target
*/

#include <tscore/debug/assert.h>
#include <tsgraphics/RenderTarget.h>

using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ImageTarget creation functions
//////////////////////////////////////////////////////////////////////////////////////////////////////

ImageTarget ImageTarget::createColourTarget(RenderDevice* device, uint32 width, uint32 height, ImageFormat format, uint32 multisampleLevel, bool shaderVisible)
{
	tsassert(device);

	ImageTarget target;
	ImageResourceInfo& info = target.m_info;

	info.width = width;
	info.height = height;
	info.type = ImageType::_2D;
	info.usage = ImageUsage::RTV;
	info.format = format;
	info.msLevels = multisampleLevel;

	if (shaderVisible)
		info.usage |= ImageUsage::SRV;

	target.swap(device->createResourceImage(nullptr, info));
	return std::move(target);
}

ImageTarget ImageTarget::createDepthTarget(RenderDevice* device, uint32 width, uint32 height, ImageFormat format, uint32 multisampleLevel)
{
	tsassert(device);

	ImageTarget target;
	ImageResourceInfo& info = target.m_info;

	info.width = width;
	info.height = height;
	info.type = ImageType::_2D;
	info.usage = ImageUsage::DSV;
	info.format = format;
	info.msLevels = multisampleLevel;

	target.swap(device->createResourceImage(nullptr, info));
	return std::move(target);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Resize the given image target
//////////////////////////////////////////////////////////////////////////////////////////////////////

void ImageTarget::resize(uint32 width, uint32 height)
{
	tsassert(Base::device());

	m_info.width = width;
	m_info.height = height;

	Base::swap(Base::device()->createResourceImage(nullptr, m_info, Base::release()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
