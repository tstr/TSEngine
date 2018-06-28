/*
    Image object
*/

#include <fstream>

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#include <tsgraphics/Image.h>
#include <tsgraphics/schemas/Image.rcs.h>

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

void Image::loadEmpty(RenderDevice* device)
{
	m_img = device->createEmptyResource(m_img.release());
}

bool Image::load(RenderDevice* device, const String& imageFile)
{
	using std::ifstream;
	using std::ios;

	rc::ResourceLoader loader;

	tsinfo("Loading image: \"%\"", imageFile);

	loader.load(ifstream(imageFile, ios::binary));

	if (loader.fail())
	{
		tswarn("Unable to load image.");
		return !setError(true);
	}

	const auto& imgReader = loader.deserialize<tsr::Image>();

	const uint32 sign = imgReader.signature();
	tsassert(memcmp("TSTX", reinterpret_cast<const char*>(&sign), 4) == 0);

	ImageResourceInfo info;
	ResourceData data;

	data.memoryByteDepth = imgReader.byteDepth();
	data.memoryByteWidth = imgReader.byteWidth();
	data.memory = imgReader.data().data();

	info.format = (ImageFormat)imgReader.format();
	info.type = (ImageType)imgReader.type();
	info.usage = ImageUsage::SRV;
	info.height = imgReader.height();
	info.width = imgReader.width();
	info.length = imgReader.length();
	info.useMips = true;
	info.mipLevels = 1;
	info.msLevels = 1;

	//Create image resource
	m_img = device->createResourceImage(&data, info, m_img.release());

	if (!m_img)
	{
		tserror("Unable to create image.");
		return !setError(true);
	}

	//Save image info
	m_imgInfo = info;

	return !setError(false);
}

///////////////////////////////////////////////////////////////////////////////

ImageView Image::getView2D(uint32 index)
{
	ImageView img;
	img.image = handle();
	img.index = index;
	img.count = 1;
	img.type = ImageType::_2D;
	return img;
}

ImageView Image::getViewArray(uint32 start, uint32 count)
{
	ImageView img;
	img.image = handle();
	img.index = start;
	img.count = count;
	img.type = ImageType::_2D;
	return img;
}

///////////////////////////////////////////////////////////////////////////////
