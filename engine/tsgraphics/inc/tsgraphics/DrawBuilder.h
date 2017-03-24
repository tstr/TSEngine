/*
	Draw command builder helper class
*/

#pragma once

#include <tsgraphics/GraphicsSystem.h>

namespace ts
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
		Draw Command builder class
	*/
	class CDrawBuilder
	{
	private:

		GraphicsSystem* m_system;
		SDrawCommand m_command;

	public:

		CDrawBuilder(GraphicsSystem* system) : m_system(system) {}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// State setting methods
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		//Set shader program
		void setShader(ShaderId id)
		{
			SShaderProgram prog;
			m_system->getShaderManager()->getProgram(id, prog);
			m_command.shaderVertex = prog.hVertex;
			m_command.shaderPixel = prog.hPixel;
			m_command.shaderGeometry = prog.hGeometry;
			m_command.shaderDomain = prog.hDomain;
			m_command.shaderHull = prog.hHull;
		}

		void setShader(const char* name)
		{
			ShaderId id;
			m_system->getShaderManager()->load(name, id);
			this->setShader(id);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set resources
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		void setTexture(uint32 slot, TextureId id)
		{
			STextureProperties props;
			HTexture tex;
			m_system->getTextureManager()->getTexProperties(id, props);
			m_system->getTextureManager()->getTexHandle(id, tex);

			STextureUnit unit;
			unit.texture = tex;
			unit.textureType = props.type;
			unit.arrayIndex = 0;
			unit.arrayCount = props.arraySize;

			this->setTexture(slot, unit);
		}

		void setTexture(uint32 slot, const STextureUnit& texUnit)
		{
			m_command.textureUnits[slot] = texUnit;
		}

		void setConstantBuffer(uint32 slot, HBuffer cb)
		{
			m_command.constantBuffers[slot] = cb;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set mesh
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		void setVertexBuffer(uint32 slot, HBuffer vb, uint32 stride, uint32 offset, const SVertexAttribute* attribs, uint32 attribCount)
		{
			m_command.vertexBuffers[slot] = vb;
			m_command.vertexStrides[slot] = stride;
			m_command.vertexOffsets[slot] = offset;

			const uint32 aoffset = m_command.vertexAttribCount;

			for (uint32 i = 0; i < attribCount; i++)
			{
				m_command.vertexAttribs[i + aoffset] = attribs[i];
				m_command.vertexAttribCount++;
			}
		}

		void setIndexBuffer(HBuffer ib)
		{
			m_command.indexBuffer = ib;
		}

		void setVertexTopology(EVertexTopology topology)
		{
			m_command.vertexTopology = topology;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set render states
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		void setBlendState(const SBlendState& state)
		{
			m_command.blendState = state;
		}

		void setRasterState(const SRasterState& state)
		{
			m_command.rasterState = state;
		}

		void setDepthState(const SDepthState& state)
		{
			m_command.depthState = state;
		}

		void setTextureSampler(uint32 slot, const STextureSampler& state)
		{
			m_command.textureSamplers[slot] = state;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set draw type
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		void setDraw(int32 base, uint32 start, uint32 count)
		{
			m_command.mode = EDrawMode::eDraw;
			m_command.vertexBase = base;
			m_command.vertexStart = start;
			m_command.vertexCount = count;
		}

		void setDrawIndexed(int32 base, uint32 istart, uint32 icount)
		{
			m_command.mode = EDrawMode::eDrawIndexed;
			m_command.vertexBase = base;
			m_command.indexStart = istart;
			m_command.indexCount = icount;
		}

		void setDrawInstanced(int32 base, uint32 start, uint32 count, uint32 instances)
		{
			m_command.mode = EDrawMode::eDrawInstanced;
			m_command.vertexBase = base;
			m_command.vertexStart = start;
			m_command.vertexCount = count;
			m_command.instanceCount = instances;
		}

		void setDrawIndexedInstanced(int32 base, uint32 istart, uint32 icount, uint32 instances)
		{
			m_command.mode = EDrawMode::eDrawInstanced;
			m_command.vertexBase = base;
			m_command.indexStart = istart;
			m_command.indexCount = icount;
			m_command.instanceCount = instances;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		
		void get(SDrawCommand& cmd) const
		{
			cmd = m_command;
		}
		
		void reset()
		{
			m_command = SDrawCommand();
		}
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}