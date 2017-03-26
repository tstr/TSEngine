/*
	Graphics Context
*/

#pragma once

#include <tsgraphics/GraphicsSystem.h>

#include <tsgraphics/BufferPool.h>
#include <tsgraphics/ShaderManager.h>
#include <tsgraphics/TextureManager.h>
#include <tsgraphics/MeshManager.h>

#include <vector>

namespace ts
{
	class CDrawBuilder;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Graphics Context class
	*/
	class GraphicsContext
	{
	private:

		GraphicsSystem* m_system;
		
		CShaderManager m_shaderManager;
		CTextureManager m_textureManager;
		CMeshManager m_meshManager;
		CBufferPool m_bufferPool;

		std::vector<HDrawCmd> m_drawPool;
		CommandQueue m_drawQueue;

	public:

		//Get resource managers
		CMeshManager* getMeshManager() { return &m_meshManager; }
		CTextureManager* getTextureManager() { return &m_textureManager; }
		CShaderManager* getShaderManager() { return &m_shaderManager; }
		CBufferPool* getPool() { return &m_bufferPool; }
		//Get render queue
		CommandQueue* getQueue() { return &m_drawQueue; }

		//Ctor/dtor
		TSGRAPHICS_API GraphicsContext(GraphicsSystem* system);
		TSGRAPHICS_API ~GraphicsContext();

		//Get parent system
		GraphicsSystem* const getSystem() const { return m_system; }

		//Draw command methods
		TSGRAPHICS_API int createDraw(const CDrawBuilder& build, HDrawCmd& cmd);
		TSGRAPHICS_API int destroyDraw(HDrawCmd cmd);
		TSGRAPHICS_API void clearDraws();

		//Commit queued draws on this context
		TSGRAPHICS_API void commit();
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Draw Command builder class
	*/
	class CDrawBuilder
	{
	private:

		GraphicsContext* m_context;
		SDrawCommand m_command;

	public:

		CDrawBuilder(GraphicsContext* context) : m_context(context) {}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// State setting methods
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		//Set shader program
		void setShader(ShaderId id)
		{
			SShaderProgram prog;
			m_context->getShaderManager()->getProgram(id, prog);
			m_command.shaderVertex = prog.hVertex;
			m_command.shaderPixel = prog.hPixel;
			m_command.shaderGeometry = prog.hGeometry;
			m_command.shaderDomain = prog.hDomain;
			m_command.shaderHull = prog.hHull;
		}

		void setShader(const char* name)
		{
			ShaderId id;
			m_context->getShaderManager()->load(name, id);
			this->setShader(id);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// Set resources
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		void setTexture(uint32 slot, TextureId id)
		{
			STextureProperties props;
			HTexture tex;
			m_context->getTextureManager()->getTexProperties(id, props);
			m_context->getTextureManager()->getTexHandle(id, tex);

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
		// Set mesh params
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

		void setMesh(MeshId mesh, uint32 firstSlot = 0)
		{
			SMeshInstance meshInst;

			if (m_context->getMeshManager()->getMeshInstance(mesh, meshInst))
			{
				return;
			}

			this->setVertexBuffer(firstSlot, meshInst.vertexBuffers[0], meshInst.vertexStrides[0], meshInst.vertexOffset[0], meshInst.vertexAttributes, meshInst.vertexAttributeCount);
			this->setVertexTopology(meshInst.topology);
			this->setIndexBuffer(meshInst.indexBuffer);
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
