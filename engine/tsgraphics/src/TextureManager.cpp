/*
	Texture management source
*/

#include <tsgraphics/api/RenderApi.h>
#include <tsgraphics/texturemanager.h>

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/filesystem/pathhelpers.h>

#include <tscore/table.h>

#include "TextureImporter.h"

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Texture classes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TextureInstance
{
private:

	GraphicsCore* m_pCore = nullptr;
	HTexture m_hTex = HTEXTURE_NULL;
	STextureResourceDesc m_texDesc;
	
public:

	HTexture getResource() const
	{
		return m_hTex;
	}

	STextureResourceDesc getResourceDesc() const
	{
		return m_texDesc;
	}

	TextureInstance() {}
	explicit TextureInstance(const TextureInstance& inst) = delete;
	
	TextureInstance(TextureInstance&& rhs)
	{
		auto pCore = this->m_pCore;
		auto hTex = this->m_hTex;
		auto texDesc = this->m_texDesc;

		this->m_pCore = rhs.m_pCore;
		this->m_hTex = rhs.m_hTex;
		this->m_texDesc = rhs.m_texDesc;

		rhs.m_pCore = pCore;
		rhs.m_hTex = hTex;
		rhs.m_texDesc = texDesc;
	}

	TextureInstance(GraphicsCore* core, HTexture tex, const STextureResourceDesc& desc) :
		m_pCore(core),
		m_hTex(tex),
		m_texDesc(desc)
	{}

	~TextureInstance()
	{
		if (m_pCore)
		{
			if (IRender* api = m_pCore->getApi())
			{
				if (m_hTex != HTEXTURE_NULL)
				{
					api->destroyTexture(m_hTex);
					m_hTex = HTEXTURE_NULL;
					m_pCore = nullptr;
				}
			}
		}
	}

	TextureInstance& operator=(const TextureInstance&) = delete;

	TextureInstance& operator=(TextureInstance&& rhs)
	{
		auto pCore = this->m_pCore;
		auto hTex = this->m_hTex;
		auto texDesc = this->m_texDesc;

		this->m_pCore = rhs.m_pCore;
		this->m_hTex = rhs.m_hTex;
		this->m_texDesc = rhs.m_texDesc;

		rhs.m_pCore = pCore;
		rhs.m_hTex = hTex;
		rhs.m_texDesc = texDesc;

		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CTextureManager::Manager
{
	GraphicsCore* system = nullptr;
	Path rootPath;

	Table<TextureInstance, TextureId> texturePool;
	CTextureImporter textureImport;

	//Create and cache a texture resource
	ETextureManagerStatus create(TextureId& id, const STextureResourceData* data, const STextureResourceDesc& desc)
	{
		IRender* api = system->getApi();
		HTexture hTex = HTEXTURE_NULL;

		if (!api || api->createResourceTexture(hTex, data, desc))
		{
			return eTextureManagerStatus_Fail;
		}

		texturePool.create(TextureInstance(system, hTex, desc), id);

		return eTextureManagerStatus_Ok;
	}

	//Release texture instance
	void destroy(TextureId id)
	{
		texturePool.destroy(id);
	}

	//Load texture instance from a file
	ETextureManagerStatus load(const Path& path, TextureId& id, int flags)
	{
		Path filepath(path);
		resolvePath(filepath);

		STextureLoadInfo info;
		
		textureImport.load(filepath, info);

		vector<STextureResourceData> dataDesc(info.desc.arraySize);
		for (uint32 i = 0; i < info.desc.arraySize; i++)
		{
			dataDesc[i].memory = (const byte*)info.data + (i * (info.desc.width * info.desc.height * info.byteWidth));
			dataDesc[i].memoryByteDepth = info.byteDepth;
			dataDesc[i].memoryByteWidth = info.byteWidth;
		}

		ETextureManagerStatus status = this->create(id, &dataDesc[0], info.desc);

		textureImport.unload(info);

		return status;
	}

	//Resolve path to texture file - can be absolute or relative to texture root directory
	bool resolvePath(Path& texpath)
	{
		if (isAbsolutePath(texpath) && isFile(texpath))
		{
			return true;
		}
		else
		{
			Path p = this->rootPath;
			p.addDirectories(texpath);
			
			if (isFile(p))
			{
				texpath = p;
				return true;
			}
		}

		texpath = "";
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ctor/dtor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

CTextureManager::CTextureManager(GraphicsCore* system, const Path& rootpath) :
	pManage(new Manager())
{
	pManage->system = system;
	pManage->rootPath = rootpath;
}

CTextureManager::~CTextureManager()
{
	if (pManage)
	{
		clear();
	}

	//Force release the internal implementation - otherwise memory leak
	pManage.reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

ETextureManagerStatus CTextureManager::load(const Path& path, TextureId& id, int flags)
{
	if (!pManage)
	{
		return eTextureManagerStatus_NullManager;
	}

	return pManage->load(path, id, flags);
}

ETextureManagerStatus CTextureManager::create(TextureId& id, const STextureResourceData* data, const STextureResourceDesc& desc)
{
	if (!pManage)
	{
		return eTextureManagerStatus_NullManager;
	}

	return pManage->create(id, data, desc);
}

void CTextureManager::destroy(TextureId id)
{
	tsassert(pManage);
	pManage->destroy(id);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTextureManager::clear()
{
	tsassert(pManage);
	pManage->texturePool.clear();
}

void CTextureManager::setRootpath(const Path& texturepath)
{
	if (pManage)
	{
		pManage->rootPath = texturepath;
	}
}

Path CTextureManager::getRootpath() const
{
	if (pManage)
	{
		return pManage->rootPath;
	}

	return Path("");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Texture properties
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTextureManager::getTexPath(TextureId id, Path& path)
{
	tsassert(pManage);

	path = "";
}

void CTextureManager::getTexProperties(TextureId id, STextureProperties& props)
{
	tsassert(pManage);

	const TextureInstance& inst = pManage->texturePool.get(id);

	STextureResourceDesc desc = inst.getResourceDesc();

	props.arraySize = desc.arraySize;
	props.format = desc.texformat;
	props.depth = desc.depth;
	props.height = desc.height;
	props.width = desc.width;
	props.mipLevels = 1;
	props.type = desc.textype;
}

void CTextureManager::getTexHandle(TextureId id, HTexture& hTex)
{
	tsassert(pManage);

	const TextureInstance& inst = pManage->texturePool.get(id);
	hTex = inst.getResource();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
