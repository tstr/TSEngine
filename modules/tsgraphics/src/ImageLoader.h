/*
	Image loader header
*/

#pragma once

#include <tscore/path.h>
#include <tsgraphics/Defs.h>

#include <vector>

#include <rcschema.h>

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct ImageLoadInfo
	{
		ImageResourceInfo desc;
		uint32 byteWidth = 0;
		uint32 byteDepth = 0;
		const void* data = nullptr;
	};

	/*
		Image loader class:

		Loads image information from a file.
		Is singlethreaded.
	*/
	class ImageLoader
	{
	private:

		rc::ResourceLoader m_rcloader;

	public:

		ImageLoader();
		~ImageLoader();

		//Load a texture from a given file
		int load(const Path& file, ImageLoadInfo& info);
		//Unload a texture
		void unload(ImageLoadInfo& info);
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////
