/*
	Graphics Context
*/

#pragma once

#include <tsgraphics/GraphicsSystem.h>
#include <tsgraphics/CommandQueue.h>
#include <tsgraphics/BufferPool.h>
#include <tsgraphics/MeshManager.h>
#include <tsgraphics/DrawBuilder.h>

#include <vector>

namespace ts
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	class CDrawBuilder;
	class GraphicsSystem;

	/*
		Graphics Context class
	*/
	class GraphicsContext
	{
	private:

		GraphicsSystem* m_system;
		
		CMeshManager m_meshManager;
		CBufferPool m_bufferPool;

		std::vector<HDrawCmd> m_drawPool;
		CommandQueue m_drawQueue;

	protected:

		CBufferPool* getPool() { return &m_bufferPool; }
		CMeshManager* getMeshManager() { return &m_meshManager; }
		CommandQueue* getQueue() { return &m_drawQueue; }

	public:
		
		GraphicsContext(GraphicsSystem* system) :
			m_system(system)
		{
			m_meshManager = CMeshManager(m_system);
			m_bufferPool = CBufferPool(m_system);
			m_drawQueue = CommandQueue(1024);
		}
		
		~GraphicsContext()
		{
			clearDraws();
		}

		GraphicsSystem* const getSystem() const { return m_system; }

		//Draw command methods
		TSGRAPHICS_API int createDraw(const CDrawBuilder& build, HDrawCmd& cmd);
		TSGRAPHICS_API int destroyDraw(HDrawCmd cmd);
		TSGRAPHICS_API void clearDraws();

		//Interface methods
		virtual CommandQueue* render(HTarget display) = 0;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
