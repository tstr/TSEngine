/*
	Model importer class 
*/

#include "modelimporter.h"

#include <fstream>
#include <tscore/debug/log.h>
#include <tscore/debug/assert.h>
#include <tscore/system/time.h>
#include <tsengine/configfile.h>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////
//helpers

inline string getKey(string matname, string matproperty)
{
	return (matname + "." + matproperty);
}

inline Vector getVectorProperty(string value)
{
	if (trim(value) == "")
		return Vector();

	Vector v;
	vector<string> tokens = split(value, ',');

	if (tokens.size() > 0) v.x() = stof(tokens[0]);
	if (tokens.size() > 1) v.y() = stof(tokens[1]);
	if (tokens.size() > 2) v.z() = stof(tokens[2]);
	if (tokens.size() > 3) v.w() = stof(tokens[3]);

	return v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CModel::CModel(CRenderModule* rendermodule, const Path& path) :
	m_rendermodule(rendermodule)
{
	tsassert(m_rendermodule);
	import(path);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Import function
/////////////////////////////////////////////////////////////////////////////////////////////////

bool CModel::import(const Path& path)
{
	tsassert(m_rendermodule);
	
	if (!ifstream(path.str()).good())
	{
		tswarn("file does not exist");
		return false;
	}
	
	//
	//	Model
	//
	
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
	
	vector<SModelMesh> meshes(header.numMeshes);
	vector<SModelVertex> vertices(header.numVertices);
	vector<ModelIndex> indices(header.numIndices);
	
	modelfile.read(reinterpret_cast<char*>(&meshes[0]), sizeof(SModelMesh) * meshes.size());
	modelfile.read(reinterpret_cast<char*>(&vertices[0]), sizeof(SModelVertex) * vertices.size());
	modelfile.read(reinterpret_cast<char*>(&indices[0]), sizeof(ModelIndex) * indices.size());
	
	m_vertices = CVertexBuffer(m_rendermodule, &vertices[0], (uint32)vertices.size());
	m_indices = CIndexBuffer(m_rendermodule, &indices[0], (uint32)indices.size());
	
	if (modelfile.fail())
	{
		tswarn("could not read model data");
		return false;
	}
	
	t.stop();
	
	tsinfo("model imported successfully (%ms)", t.deltaTime());
	
	//
	//	Materials
	//
	
	string materialpathname(m_filepath.str());
	materialpathname.erase(materialpathname.find_last_of('.'), string::npos);
	materialpathname += ".tmat";
	Path materialpath(materialpathname);
	Path materialroot(materialpath.getParent());
	
	tsinfo("creating materials \"%\"...", materialpath.str());
	
	//INI parser
	ConfigFile matfile(materialpath.str());
	
	size_t idx = 0;

	int32 basevertex = 0;

	//Iterator over model meshes
	for (SModelMesh& fmesh : meshes)
	{
		SMesh mesh;
		mesh.indexOffset = fmesh.indexOffset;
		mesh.indexCount = fmesh.indexCount;
		mesh.vertexBase = basevertex;
		basevertex += fmesh.numVertices;

		const char* mname = fmesh.materialName.str();

		tsinfo("material(%) : %", idx, mname);

		matfile.getProperty(getKey(mname, "alpha"), mesh.material.alpha);
		matfile.getProperty(getKey(mname, "shininess"), mesh.material.shininess);

		string buf;
		matfile.getProperty(getKey(mname, "diffuseColour"), buf);
		mesh.material.diffuseColour = getVectorProperty(buf); buf = "";
		matfile.getProperty(getKey(mname, "ambientColour"), buf);
		mesh.material.ambientColour = getVectorProperty(buf); buf = "";
		matfile.getProperty(getKey(mname, "emissiveColour"), buf);
		mesh.material.emissiveColour = getVectorProperty(buf); buf = "";

		buf = "";
		matfile.getProperty(getKey(mname, "diffuseMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			m_rendermodule->getTextureManager().loadTexture2D(texpath, mesh.material.diffuseMap);
		}

		buf = "";
		matfile.getProperty(getKey(mname, "normalMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			m_rendermodule->getTextureManager().loadTexture2D(texpath, mesh.material.normalMap);
		}

		buf = "";
		matfile.getProperty(getKey(mname, "specularMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			m_rendermodule->getTextureManager().loadTexture2D(texpath, mesh.material.specularMap);
		}

		buf = "";
		matfile.getProperty(getKey(mname, "displacementMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			m_rendermodule->getTextureManager().loadTexture2D(texpath, mesh.material.displacementMap);
		}

		buf = "";
		matfile.getProperty(getKey(mname, "ambientMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			m_rendermodule->getTextureManager().loadTexture2D(texpath, mesh.material.ambientMap);
		}

		m_meshes.push_back(mesh);
		idx++;
	}

	tsinfo("materials imported successfully");

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
