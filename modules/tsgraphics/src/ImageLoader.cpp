/*
	Texture Importer source
*/

#include <fstream>

#include "ImageLoader.h"

#include <tsgraphics/Device.h>
#include <tsgraphics/schemas/Texture.rcs.h>

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

using namespace std;
using namespace ts;

enum EState
{
	eStateFree = 0,
	eStateLoad = -1,
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ctor/dtor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Load a image from a given file
int ImageLoader::load(const Path& filepath, ImageLoadInfo& info)
{
	tsinfo("Loading image: \"%\"", filepath.str());

	m_rcloader.load(ifstream(filepath.str(), ios::binary));

	if (m_rcloader.fail())
	{
		return false;
	}

	const tsr::Texture& imgReader = m_rcloader.deserialize<tsr::Texture>();

	const uint32 sign = imgReader.signature();
	tsassert(memcmp("TSTX", reinterpret_cast<const char*>(&sign), 4) == 0);

	info.byteDepth = imgReader.byteDepth();
	info.byteWidth = imgReader.byteWidth();
	info.data = imgReader.data().data();

	info.desc.format = (ImageFormat)imgReader.format();
	info.desc.type = (ImageType)imgReader.type();
	info.desc.usage = ImageUsage::SRV;
	info.desc.height = imgReader.height();
	info.desc.width = imgReader.width();
	info.desc.length = imgReader.length();
	info.desc.useMips = true;
	
	return true;
}

//Unload a texture
void ImageLoader::unload(ImageLoadInfo& info)
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
