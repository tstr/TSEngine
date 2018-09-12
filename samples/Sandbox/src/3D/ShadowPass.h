/*
	Shadow Mapping pipeline
*/

#pragma once

#include "Renderable.h"
#include <tsgraphics/Driver.h>
#include <tsgraphics/RenderTarget.h>

namespace ts
{
	class ShadowPass
	{
	public:

		ShadowPass() {}
		ShadowPass(const ShadowPass&) = delete;

		/*
			Construct a shadow map
		*/
		ShadowPass(RenderDevice* device, uint32 width, uint32 height);

		void operator=(ShadowPass&& rhs)
		{
			m_shadowTarget = std::move(rhs.m_shadowTarget);
			m_shadowBuffer = std::move(rhs.m_shadowBuffer);
			m_shadowDepth = std::move(rhs.m_shadowDepth);
		}

		/*
			Get a view of the shadow map buffer
		*/
		ImageView getView() const { return m_shadowBuffer.getView(); }

		/*
			Get shadow render target
		*/
		TargetHandle getTarget() { return m_shadowTarget.handle(); }

	private:

		//Shadow targets
		RenderTargets<> m_shadowTarget;
		ImageTarget m_shadowBuffer;
		ImageTarget m_shadowDepth;
	};
}
