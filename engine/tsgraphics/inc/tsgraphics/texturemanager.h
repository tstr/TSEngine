/*
	Texture management header
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/api/renderapi.h>
#include <tsgraphics/api/rendercommon.h>

#include <tscore/filesystem/path.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class GraphicsSystem;
	class CTextureManager;

	class CTextureCube;

	//Texture class encapsulates the a texture resource and a texture view which can be bound to the pipeline
	class CTexture2D
	{
	private:

		CTextureManager* m_manager = nullptr;
		HTexture m_hTex;

		//Properties
		uint32 m_width = 0;
		uint32 m_height = 0;

		ETextureFormat m_texformat;

	public:

		CTexture2D() {}
		TSGRAPHICS_API CTexture2D(
			CTextureManager* manager,
			const STextureResourceData& data,
			const STextureResourceDesc& desc
		);

		HTexture getHandle() const { return m_hTex; }

		uint32 getWidth() const { return m_width; }
		uint32 getHeight() const { return m_height; }
		ETextureFormat getFormat() const { return m_texformat; }
	};
	
	//Texture cube class
	class CTextureCube
	{
	private:

		CTextureManager* m_manager = nullptr;

		HTexture m_hTex;

		//Properties
		uint32 m_facewidth = 0;
		uint32 m_faceheight = 0;

		ETextureFormat m_texformat;

	public:

		CTextureCube() {}
		TSGRAPHICS_API CTextureCube(
			CTextureManager* manager,
			const STextureResourceData* data,
			const STextureResourceDesc& desc
		);

		HTexture getHandle() const { return m_hTex; }

		uint32 getWidth() const { return m_facewidth; }
		uint32 getHeight() const { return m_faceheight; }
		ETextureFormat getFormat() const { return m_texformat; }
	};

	//Texture manager class which is responsible for controlling the lifetime of textures and loading them from disk
	class CTextureManager
	{
	private:

		GraphicsSystem* m_graphics = nullptr;
		Path m_rootpath;
		
		uintptr_t m_token = 0;

	public:

		TSGRAPHICS_API CTextureManager(GraphicsSystem* system, const Path& rootpath = "");
		TSGRAPHICS_API ~CTextureManager();

		GraphicsSystem* const getSystem() const { return m_graphics; }

		void setRootpath(const Path& rootpath) { m_rootpath = rootpath; }
		Path getRootpath() const { return m_rootpath; }

		bool TSGRAPHICS_API loadTexture2D(const Path& file, CTexture2D& texture);
		bool TSGRAPHICS_API loadTextureCube(const Path& file, CTextureCube& texture);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////