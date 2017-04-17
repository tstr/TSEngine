/*
	3D Graphics class
*/

#pragma once

#include <tsgraphics/GraphicsContext.h>
#include <tscore/filesystem/pathhelpers.h>

#include "Model.h"

namespace ts
{
	class Graphics3D : private GraphicsContext
	{
	private:

		Matrix m_matrixView;
		Matrix m_matrixProj;

		HBuffer m_sceneBuffer;

	public:

		Graphics3D(GraphicsSystem* system);
		~Graphics3D();

		using GraphicsContext::getTextureManager;
		using GraphicsContext::getShaderManager;
		using GraphicsContext::getMeshManager;
		using GraphicsContext::getSystem;

		/////////////////////////////////////////////////////////

		void setScene(CRenderItemInfo& info)
		{
			info.setConstantBuffer(0, m_sceneBuffer);
			info.setShader("SandboxShader");

			STextureSampler sampler;
			sampler.addressU = ETextureAddressMode::eTextureAddressClamp;
			sampler.addressV = ETextureAddressMode::eTextureAddressClamp;
			sampler.addressW = ETextureAddressMode::eTextureAddressClamp;
			sampler.filtering = eTextureFilterAnisotropic16x;
			sampler.enabled = true;
			info.setTextureSampler(0, sampler);
		}

		void setView(Matrix view)
		{
			m_matrixView = view;
		}

		void setProjection(Matrix proj)
		{
			m_matrixProj = proj;
		}

		CModel createModel()
		{
			return CModel(this);
		}

		CRenderItemInfo createInfo()
		{
			return CRenderItemInfo(this);
		}

		CRenderItem createItem(const CRenderItemInfo& info)
		{
			return CRenderItem(this, info);
		}

		/////////////////////////////////////////////////////////

		void draw(const CRenderItem& item);

		void update();
	};
}
