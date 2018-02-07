/*
	Graphics Context
*/

#pragma once

#include <tsgraphics/GraphicsSystem.h>

#include <tsgraphics/BufferPool.h>
#include <tsgraphics/ShaderManager.h>
#include <tsgraphics/TextureManager.h>
#include <tsgraphics/MeshManager.h>

#include <tscore/table.h>

#include <vector>
#include <unordered_map>

namespace ts
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Graphics Context class:

		Encapsulates a render target and set of render commands
	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class GraphicsContext
	{
	public:

		/*
			Render Item ID
		*/
		typedef uint32 ItemID;

		//////////////////////////////////////////////////////////////////////////////////////////
		/*
			Abstract Render Pass class
		*/
		//////////////////////////////////////////////////////////////////////////////////////////
		class Pass
		{
		private:

			GraphicsContext* m_context;

			HTexture m_targetTexture;
			HTarget m_target;

			bool m_targetIsDisplay;

			struct ItemInstance
			{
				HDrawCmd cmd = HDRAWCMD_NULL;
				SDrawCommand cmdDesc;
				uint32 cmdHash;
			};

			std::vector<ItemInstance> m_itemInstances;

		protected:

			// Initializes a new 2D texture target for this render pass
			TSGRAPHICS_API void initTarget2D(uint width, uint height, uint multisampling);
			// Initializes render pass to use the Display as it's target
			TSGRAPHICS_API void initDisplayTarget();
			// Returns true if m_target is the Display
			bool isDisplayTarget() const { return m_targetIsDisplay; }

			// Set/get a rendering command
			TSGRAPHICS_API void setItemCommand(ItemID item, const SDrawCommand& command);
			TSGRAPHICS_API void getItemCommandDesc(ItemID item, SDrawCommand& command) const;
			TSGRAPHICS_API uint32 getItemCommandHash(ItemID item) const;
			TSGRAPHICS_API HDrawCmd getItemCommand(ItemID item) const;

			TSGRAPHICS_API void resetItemCommands();
			TSGRAPHICS_API void resetTargets();

		public:

			TSGRAPHICS_API Pass(GraphicsContext* context);
			TSGRAPHICS_API ~Pass();

			// Get parent context
			GraphicsContext* getContext() { return m_context; }

			// Get target for this pass
			HTarget getTarget() const { return m_target; }

			Pass(const Pass& pass) = delete;
		};

		friend class Pass;

		//////////////////////////////////////////////////////////////////////////////////////////

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

		//Render Item methods
		TSGRAPHICS_API ItemID createItem();
		TSGRAPHICS_API bool isItem(ItemID renderItem) const;

		//Commit queued draws on this context
		TSGRAPHICS_API void commit();

	private:

		//Parent system
		GraphicsSystem* m_system;

		//Command queue
		CommandQueue m_drawQueue;

		//Resource managers
		CShaderManager m_shaderManager;
		CTextureManager m_textureManager;
		CMeshManager m_meshManager;
		CBufferPool m_bufferPool;

		HandleAllocator<ItemID> m_itemAllocator;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
