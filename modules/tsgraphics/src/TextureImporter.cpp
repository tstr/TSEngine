/*
	Texture Importer source
*/

#include <fstream>

#include "TextureImporter.h"

#include <tsgraphics/api/RenderDef.h>
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

CTextureImporter::CTextureImporter()
{
}

CTextureImporter::~CTextureImporter()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Load a texture from a given file
int CTextureImporter::load(const Path& filepath, STextureLoadInfo& info)
{
	tsinfo("Loading texture: \"%\"", filepath.str());

	m_rcloader.load(ifstream(filepath.str(), ios::binary));

	if (m_rcloader.fail())
	{
		return false;
	}

	const tsr::Texture& texReader = m_rcloader.deserialize<tsr::Texture>();

	const uint32 sign = texReader.signature();
	tsassert(memcmp("TSTX", reinterpret_cast<const char*>(&sign), 4) == 0);

	info.byteDepth = texReader.byteDepth();
	info.byteWidth = texReader.byteWidth();
	info.data = texReader.data().data();

	auto c = ((const uint32*)info.data)[0];
	auto l = texReader.data().length();

	info.desc.texformat = (ETextureFormat)texReader.format();
	info.desc.textype = (ETextureResourceType)texReader.type();
	info.desc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
	info.desc.height = texReader.height();
	info.desc.width = texReader.width();
	info.desc.depth = 1;
	info.desc.arraySize = texReader.length();
	info.desc.useMips = true;
	
	return true;
}

//Unload a texture
void CTextureImporter::unload(STextureLoadInfo& info)
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
