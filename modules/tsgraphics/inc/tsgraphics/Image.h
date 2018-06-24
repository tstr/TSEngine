/*
    Image object
*/

#pragma once

#include "Driver.h"

namespace ts
{
    class Image
    {
    public:
        
		Image() {}
		Image(RenderDevice* device) { loadEmpty(device); }
		Image(RenderDevice* device, const String& imageFile) { load(device, imageFile); }
        
		void loadEmpty(RenderDevice* device);
		bool load(RenderDevice* device, const String& imageFile);

		bool loaded() const { return !m_img.null(); }
		bool error() const { return m_error; }

		ResourceHandle handle() const { return m_img.handle(); }

		ImageView getView2D(uint32 index);
		ImageView getViewArray(uint32 start, uint32 count);

    private:

		bool setError(bool err) { m_error = err; return m_error; }
		
		bool m_error = false;
        RPtr<ResourceHandle> m_img;
		ImageResourceInfo m_imgInfo;
    };
}