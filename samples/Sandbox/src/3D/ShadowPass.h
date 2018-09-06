/*
	Shadow Mapping pipeline
*/

#pragma once

#include "Renderable.h"
#include <tsgraphics/Graphics.h>
#include <tsgraphics/Shader.h>
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
		ShadowPass(GraphicsSystem* gfx, uint32 width, uint32 height);

		void operator=(ShadowPass&& rhs)
		{
			m_shadowBuffer = std::move(rhs.m_shadowBuffer);
			m_shadowDepth = std::move(rhs.m_shadowDepth);
			m_shadowShader = std::move(rhs.m_shadowShader);
			m_shadowTarget = std::move(rhs.m_shadowTarget);
		}

		/*
			Get a view of the shadow map buffer
		*/
		ImageView getView() const { return m_shadowBuffer.getView(); }

		/*
			Get shadow render target
		*/
		TargetHandle getTarget() { return m_shadowTarget.handle(); }

		/*
			Get shadowmap shader
		*/
		ShaderHandle getShader() const { return m_shadowShader.handle(); }


	private:

		//Shadow targets
		RenderTargets<> m_shadowTarget;
		ImageTarget m_shadowBuffer;
		ImageTarget m_shadowDepth;

		//Shader
		ShaderProgram m_shadowShader;
	};
}
