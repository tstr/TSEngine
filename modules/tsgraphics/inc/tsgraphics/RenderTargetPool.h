/*
    Render Target Pool class
*/

#pragma once

#include "RenderTarget.h"

namespace ts
{
	/*
		Image Target Pool:

		An image target pool represents a set of render targets with the same dimensions,

		This is useful for when you need multiple render targets of the same size while being dynamically resizable.
	*/
    class ImageTargetPool
    {
    public:

		ImageTargetPool() = default;

		/*
			Construct an image target pool of the given dimensions
		*/
        ImageTargetPool(
            RenderDevice* device,
            uint32 width,
            uint32 height,
            uint32 multisampleLevel
        ) :
			m_device(device),
			m_width(width),
			m_height(height),
			m_msLevel(multisampleLevel)
		{}
        
		/*
			Get the full viewport dimensions
		*/
        Viewport getViewport() const
        {
			Viewport viewp;
			viewp.w = m_width;
			viewp.h = m_height;
			return viewp;
        }

		/*
			Create a new colour target in this pool
		*/
        ImageView newTarget(ImageFormat format, bool shaderVisible = false)
        {
			m_targetResources.emplace_back(
				ImageTarget::createColourTarget(
					m_device,
					m_width, m_height,
					format,
					m_msLevel,
					shaderVisible
				)
			);

			return m_targetResources.back().getView();
        }
        
		/*
			Create a new depth target in this pool
		*/
        ImageView newDepthTarget(bool d32 = true)
        {
			m_targetResources.emplace_back(
				ImageTarget::createDepthTarget(
					m_device,
					m_width, m_height,
					d32 ? ImageFormat::DEPTH32 : ImageFormat::DEPTH16,
					m_msLevel
				)
			);

			return m_targetResources.back().getView();
        }
        
		/*
			Resize all allocated render targets
		*/
        void resize(uint32 width, uint32 height, uint32 multisampleLevel)
        {
			m_width = width;
			m_height = height;
			m_msLevel = multisampleLevel;

            for (auto& tgt : m_targetResources)
            {
                tgt.resize(m_width, m_height);
            }
        }
        
        void resize(uint32 width, uint32 height)
		{
			resize(width, height, m_msLevel);
		}

		void resize(DisplayConfig config)
		{
			resize(config.resolutionW, config.resolutionH, config.multisampleLevel);
		}
        
    private:
        
        RenderDevice* m_device = nullptr;
        
        uint32 m_width = 0;
        uint32 m_height = 0;
        uint32 m_msLevel = 0;
        
        std::vector<ImageTarget> m_targetResources;
    };
}
