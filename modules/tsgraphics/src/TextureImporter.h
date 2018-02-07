/*
	Texture Importer header
*/

#pragma once

#include <tscore/path.h>
#include <tsgraphics/api/RenderDef.h>

#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct STextureLoadInfo
	{
		STextureResourceDesc desc;
		uint32 byteWidth = 0;
		uint32 byteDepth = 0;
		const void* data = nullptr;
	};

	/*
		Texture importer class:

		Loads texture information from a file.
		Is singlethreaded.
	*/
	class CTextureImporter
	{
	private:

		uint64 m_token = 0;
		uint32 m_state = 0;

		std::vector<uint32> m_buffer;
		
	public:

		CTextureImporter();
		~CTextureImporter();

		//Load a texture from a given file
		int load(const Path& file, STextureLoadInfo& info);
		//Unload a texture
		void unload(STextureLoadInfo& info);
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////
