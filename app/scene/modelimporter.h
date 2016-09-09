/*
	Model importer class 
*/

#pragma once

#include <tsgraphics/rendermodule.h>
#include <tscore/filesystem/path.h>
#include <tsgraphics/model/modeldefs.h>


namespace ts
{
	class CModelImporter
	{
	private:
		
		Path m_filepath;
		
		std::vector<SModelMesh> m_meshes;
		std::vector<SModelVertex> m_vertices;
		std::vector<ModelIndex> m_indices;
		
	public:
		
		CModelImporter() {}
		
		CModelImporter(const Path& path)
		{
			import(path);
		}
		
		~CModelImporter() {}
		
		bool import(const Path& path);
		
		uint32 getMeshCount() const { return (uint32)m_meshes.size(); }
		const SModelMesh& getMesh(uint32 idx) const { return m_meshes.at(idx); }
		
		void getVertexBuffer(uint32 meshindex, CRenderModule* module, CVertexBuffer& vbuffer) const;
		void getIndexBuffer(uint32 meshindex, CRenderModule* module, CIndexBuffer& ibuffer) const;
	};
}
