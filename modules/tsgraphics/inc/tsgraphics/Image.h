/*
    Image object
*/

#pragma once

#include "Driver.h"
#include "AssetCache.h"

namespace ts
{
    class Image
    {
    public:
        
		Image() {}
		Image(RenderDevice* device) { loadEmpty(device); }
		Image(RenderDevice* device, const String& imageFile) { load(device, imageFile); }
        
		Image(const Image&) = delete;

		Image(Image&& rhs)
		{
			std::swap(m_error, rhs.m_error);
			std::swap(m_img, rhs.m_img);
			std::swap(m_imgInfo, rhs.m_imgInfo);
		}

		void loadEmpty(RenderDevice* device);
		bool load(RenderDevice* device, const String& imageFile);

		bool loaded() const { return !m_img.null(); }
		bool error() const { return m_error; }

		ResourceHandle handle() const { return m_img.handle(); }

		ImageView getView2D(uint32 index) const;
		ImageView getViewArray(uint32 start, uint32 count) const;

    private:

		bool setError(bool err) { m_error = err; return m_error; }
		
		bool m_error = false;
        RPtr<ResourceHandle> m_img;
		ImageResourceInfo m_imgInfo;
    };

	/*
		Image loader cache
	*/
	class ImageCache : public AssetCache<ImageCache, Image>
	{
	public:

		ImageCache(RenderDevice* device = nullptr) : m_device(device) {}

		Image load(const Path& filePath)
		{
			Image img;
			img.load(m_device, filePath.str());
			return std::move(img);
		}

	private:

		RenderDevice * m_device;
	};
}