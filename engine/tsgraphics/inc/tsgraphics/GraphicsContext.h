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
#include <unordered_map>

namespace ts
{
	class CRenderItem;
	class CRenderItemInfo;
	class GraphicsContext;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	typedef uint64 RenderItemID;

	/*
		Render Pass class
	*/
	class RenderPass
	{
	private:

		HTarget m_target = HTARGET_NULL;
		STargetDesc m_targetDesc;

		bool m_owns = false;

		SViewport m_targetViewport;
		SViewport m_targetScissor;

	public:

		RenderPass() : m_owns(false) {}
		RenderPass(HTexture target, const STargetDesc& desc);
		RenderPass(HTexture target, const STargetDesc& desc, const SViewport& view, const SViewport& scissor);
		RenderPass(RenderPass& pass, const SViewport& view, const SViewport& scissor);

		~RenderPass();

		HTarget getTarget() const { return m_target; }
		SViewport getView() const { return m_targetViewport; }
		SViewport getScissor() const { return m_targetScissor; }
	};

	/*
		Render View class
	*/
	class RenderView
	{
	private:
		
		struct Item
		{
			HDrawCmd cmd;
			SDrawCommand desc;
		};

		//HandleMap<RenderItemID, Item> m_itemCommands; //(m_itemCommands(&handleAllocator)

		//VectorSet<>
		std::vector<RenderPass*> m_passes;
		CommandQueue m_queue;

	protected:

		void addPass(RenderPass* pass, RenderPass* prevPass);

	public:

		//composed of RenderPasses
		RenderView(GraphicsContext* context);

		~RenderView();

		int setItemCommand(RenderItemID id, const SDrawCommand& command);
		int setItemCommand(RenderItemID id, const SDrawCommand& command);
		void submitItem(RenderItemID id);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Graphics Context class
	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class GraphicsContext
	{
	private:

		GraphicsSystem* m_system;
		
		CShaderManager m_shaderManager;
		CTextureManager m_textureManager;
		CMeshManager m_meshManager;
		CBufferPool m_bufferPool;

		std::vector<HDrawCmd> m_drawPool;
		std::map<std::string, uint32> m_passIdMapper;
		uint32 m_passIdCounter = 0;

		CommandQueue m_drawQueue;

		//Draw command methods
		TSGRAPHICS_API int allocDraw(const SDrawCommand& cmdDesc, HDrawCmd& cmd);
		TSGRAPHICS_API int freeDraw(HDrawCmd cmd);
		TSGRAPHICS_API void clearDraws();

	public:

		friend class CRenderItem;

		//Ctor/dtor
		TSGRAPHICS_API GraphicsContext(GraphicsSystem* system);
		TSGRAPHICS_API ~GraphicsContext();

		//Get parent system
		GraphicsSystem* const getSystem() const { return m_system; }

		//Get resource managers
		CMeshManager* getMeshManager() { return &m_meshManager; }
		CTextureManager* getTextureManager() { return &m_textureManager; }
		CShaderManager* getShaderManager() { return &m_shaderManager; }
		CBufferPool* getPool() { return &m_bufferPool; }
		//Get render queue
		CommandQueue* getQueue() { return &m_drawQueue; }

		//Commit queued draws on this context
		TSGRAPHICS_API void commit();
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Render Item info class - describes how a render item is created
	*/
	class CRenderItemInfo
	{
	private:

		GraphicsContext* m_context;
		SDrawCommand m_command;

	public:

		CRenderItemInfo() {}
		CRenderItemInfo(GraphicsContext* context) : m_context(context) {}

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
	/*
		Render Item class - encapsulates a set of commands
	*/
	class CRenderItem
	{
	private:

		GraphicsContext* m_context;
		HDrawCmd m_command;
		CRenderItemInfo m_commandInfo;

	public:

		CRenderItem() :
			m_context(nullptr),
			m_command(HDRAWCMD_NULL)
		{}

		TSGRAPHICS_API CRenderItem(GraphicsContext* context, const CRenderItemInfo& info);

		TSGRAPHICS_API CRenderItem(CRenderItem&& rhs);
		TSGRAPHICS_API ~CRenderItem();

		CRenderItem(const CRenderItem&) = delete;
		TSGRAPHICS_API CRenderItem& operator=(CRenderItem&& rhs);

		HDrawCmd getCommand() const { return m_command; }

		TSGRAPHICS_API void clear();

		void getInfo(CRenderItemInfo& info) const { info = m_commandInfo; }
		TSGRAPHICS_API void setInfo(const CRenderItemInfo& info);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
