/*
	Texture management header
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/GraphicsCore.h>

#include <tscore/filesystem/path.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	typedef uint32 TextureId;
	typedef uint32 Texel;

	struct STextureProperties
	{
		Texel width = 0;
		Texel height = 0;
		Texel depth = 0;

		uint32 arraySize = 0;
		uint32 mipLevels = 0;
		
		ETextureFormat format;
		ETextureResourceType type;
	};
	
	enum ETextureLoadFlags
	{
		eTextureLoadFlag_None    = 0,
		eTextureLoadFlag_GenMips = 1
	};

	enum ETextureManagerStatus
	{
		eTextureManagerStatus_Ok			 = 0,
		eTextureManagerStatus_Fail			 = 1,
		eTextureManagerStatus_NullManager	 = 2,
		eTextureManagerStatus_FileNotFound	 = 3,
		eTextureManagerStatus_FileCorrupt	 = 4,
		eTextureManagerStatus_InvalidOptions = 5,
	};

	class CTextureManager
	{
	private:

		struct Manager;
		OpaquePtr<Manager> pManage;

	public:

		OPAQUE_PTR(CTextureManager, pManage)

		CTextureManager() {}

		TSGRAPHICS_API CTextureManager(GraphicsCore* system, const Path& rootpath);
		TSGRAPHICS_API ~CTextureManager();

		TSGRAPHICS_API void setRootpath(const Path& texturepath);
		TSGRAPHICS_API Path getRootpath() const;
		
		TSGRAPHICS_API ETextureManagerStatus load(const Path& path, TextureId& id, int flags);
		TSGRAPHICS_API ETextureManagerStatus create(TextureId& id, const STextureResourceData* data, const STextureResourceDesc& desc);

		TSGRAPHICS_API void getTexPath(TextureId id, Path& path);
		TSGRAPHICS_API void getTexProperties(TextureId id, STextureProperties& props);
		TSGRAPHICS_API void getTexHandle(TextureId id, HTexture& hTex);

		TSGRAPHICS_API void destroy(TextureId id);
		TSGRAPHICS_API void clear();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////