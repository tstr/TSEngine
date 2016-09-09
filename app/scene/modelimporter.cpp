/*
	Model importer class 
*/

#include "modelimporter.h"

#include <fstream>
#include <tscore/debug/log.h>
#include <tscore/debug/assert.h>
#include <tscore/system/time.h>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////
//Import function
/////////////////////////////////////////////////////////////////////////////////////////////////

bool CModelImporter::import(const Path& path)
{
	if (!ifstream(path.str()).good())
	{
		tswarn("file does not exist");
		return false;
	}
	
	m_filepath = path;
	
	tsinfo("importing model \"%\"...", m_filepath.str());
	
	Stopwatch t;
	t.start();
	
	ifstream modelfile(m_filepath.str(), ios::binary);
	
	SModelHeader header;
	modelfile.read(reinterpret_cast<char*>(&header), sizeof(SModelHeader));
	
	if (modelfile.fail())
	{
		tswarn("could not read model header");
		return false;
	}
	
	m_meshes.resize(header.numMeshes);
	m_vertices.resize(header.numVertices);
	m_indices.resize(header.numIndices);
	
	modelfile.read(reinterpret_cast<char*>(&m_meshes[0]), sizeof(SModelMesh) * m_meshes.size());
	modelfile.read(reinterpret_cast<char*>(&m_vertices[0]), sizeof(SModelVertex) * m_vertices.size());
	modelfile.read(reinterpret_cast<char*>(&m_indices[0]), sizeof(ModelIndex) * m_indices.size());
	
	if (modelfile.fail())
	{
		tswarn("could not read model data");
		return false;
	}
	
	t.stop();
	
	tsinfo("model imported successfully (%ms)", t.deltaTime());
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CModelImporter::getVertexBuffer(uint32 meshindex, CRenderModule* module, CVertexBuffer& vbuffer) const
{
	tsassert(module);
	vbuffer = CVertexBuffer(module, &m_vertices[0], (uint32)m_vertices.size());
}

void CModelImporter::getIndexBuffer(uint32 meshindex, CRenderModule* module, CIndexBuffer& ibuffer) const
{
	tsassert(module);
	ibuffer = CIndexBuffer(module, &m_indices[0], (uint32)m_indices.size());
}

/////////////////////////////////////////////////////////////////////////////////////////////////
