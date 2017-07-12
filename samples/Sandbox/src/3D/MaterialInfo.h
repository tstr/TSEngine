/*
	Material Info structure
*/

#pragma once

#include <tscore/system/memory.h>
#include <tsgraphics/GraphicsContext.h>

namespace ts
{
	/*
		MaterialStates class encapsulates a set of material states: shader program/rendering states
	*/
	class MaterialStates
	{
	public:
		
		/*
			Shader program
		*/
		void setShader(ShaderId program)
		{
			m_shader = program;
		}

		ShaderId getShader() const
		{
			return m_shader;
		}

		/*
			Set pipeline states
		*/
		void setCullMode(ECullMode culling) { m_rasterState.cullMode = culling; }
		void setFillMode(EFillMode filling) { m_rasterState.fillMode = filling; }
		void enableDepth(bool enable) { m_depthState.enableDepth = enable; }
		void enableStencil(bool enable) { m_depthState.enableDepth = enable; }
		void enableAlpha(bool enable) { m_blendState.enable = enable; }
		void enableScissor(bool enable) { m_rasterState.enableScissor = enable; }
		void setSamplerAddressMode(ETextureAddressMode addr) { m_textureSampler.addressU = m_textureSampler.addressV = m_textureSampler.addressW = addr; }
		void setSamplerFiltering(ETextureFilterMode filter) { m_textureSampler.filtering = filter; }

		/*
			Get pipeline states
		*/
		void getStates(SBlendState& bs, SRasterState& rs, SDepthState& ds) const
		{
			bs = m_blendState;
			rs = m_rasterState;
			ds = m_depthState;
		}

		void getSamplerState(STextureSampler& sampler) const
		{
			sampler = m_textureSampler;
		}

	private:

		//Shader Program
		ShaderId m_shader;

		//Pipleline states
		SBlendState m_blendState;
		SRasterState m_rasterState;
		SDepthState m_depthState;

		STextureSampler m_textureSampler;
	};

	/*
		MaterialResources class encapsulates a set of Material resources: Textures/Constants etc.
	*/
	class MaterialResources
	{
	public:

		template<typename Rsc>
		using Map = std::map<uint, Rsc>;

		/*
			Set texture resources
		*/
		void setTexture(uint32 index, TextureId tex)
		{
			m_textures[index] = tex;
		}

		TextureId getTexture(uint32 index) const
		{
			Map<TextureId>::const_iterator it = m_textures.find(index);

			if (it != m_textures.end())
				return it->second;
			
			return 0;
		}

		Map<TextureId>::const_iterator beginTextureIterator() const
		{
			return m_textures.begin();
		}

		Map<TextureId>::const_iterator endTextureIterator() const
		{
			return m_textures.end();
		}

		/*
			Set material constant structure
		*/
		template<typename Param_t>
		void setConstants(const Param_t& params)
		{
			m_constants = make_buffer(params);
		}
		
		void setConstants(const MemoryBuffer& params)
		{
			m_constants = MemoryBuffer(params.pointer(), params.size());
		}

		MemoryBuffer getConstants() const
		{
			return MemoryBuffer(m_constants.pointer(), m_constants.size());
		}

	private:

		//Texture resources
		Map<TextureId> m_textures;

		//Material constant params
		MemoryBuffer m_constants;
	};
}
