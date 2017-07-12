/*
	Model Importer class
*/


#include "Model.h"

#include <fstream>
#include <tscore/debug/log.h>
#include <tscore/debug/assert.h>
#include <tscore/system/time.h>
#include <tsengine/VarTable.h>

#include "util/IniReader.h"

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

static void getAttributeList(vector<SVertexAttribute>& attribs, uint8 vertexFlags);

/////////////////////////////////////////////////////////////////////////////////////////////////

CModel::CModel(GraphicsContext* graphics) :
	m_graphics(graphics)
{
	tsassert(m_graphics);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Import function
/////////////////////////////////////////////////////////////////////////////////////////////////

bool CModel::import(const Path& path, uint8 attribMask)
{
	tsassert(m_graphics);

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
	INIReader matfile(materialpath.str());

	size_t idx = 0;

	int32 basevertex = 0;
	uint8 modelAttribMask = 0;

	//Iterate over model meshes
	for (SModelMesh& fmesh : meshes)
	{
		Selection select;

		select.submesh.indexOffset = fmesh.indexOffset;
		select.submesh.indexCount = fmesh.indexCount;
		select.submesh.vertexBase = basevertex;
		basevertex += fmesh.numVertices;

		select.submesh.vertexAttributes = fmesh.vertexAttributeMask;
		modelAttribMask |= fmesh.vertexAttributeMask;

		const char* mname = fmesh.materialName.str();

		if (!matfile.isSection(mname))
		{
			tswarn("material(%) % was not found", idx, mname);
			continue;
		}

		tsinfo("material(%) : %", idx, mname);

		float alpha = 0.0f;

		matfile.getProperty(getKey(mname, "alpha"), alpha);
		matfile.getProperty(getKey(mname, "shininess"), select.material.params.specularPower);

		string buf;
		matfile.getProperty(getKey(mname, "diffuseColour"), buf);
		select.material.params.diffuseColour = getVectorProperty(buf); buf = "";
		matfile.getProperty(getKey(mname, "ambientColour"), buf);
		select.material.params.ambientColour = getVectorProperty(buf); buf = "";
		matfile.getProperty(getKey(mname, "emissiveColour"), buf);
		select.material.params.emissiveColour = getVectorProperty(buf); buf = "";

		buf = "";
		matfile.getProperty(getKey(mname, "diffuseMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			if (m_graphics->getTextureManager()->load(texpath, select.material.diffuseMap, 0) == eOk)
				select.material.params.useDiffuseMap = true;
		}

		buf = "";
		matfile.getProperty(getKey(mname, "normalMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			if (m_graphics->getTextureManager()->load(texpath, select.material.normalMap, 0) == eOk)
				select.material.params.useNormalMap = true;
		}

		buf = "";
		matfile.getProperty(getKey(mname, "specularMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			if (m_graphics->getTextureManager()->load(texpath, select.material.specularMap, 0) == eOk)
				select.material.params.useSpecularMap = true;
		}

		buf = "";
		matfile.getProperty(getKey(mname, "displacementMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			if (m_graphics->getTextureManager()->load(texpath, select.material.displacementMap, 0) == eOk)
				select.material.params.useDisplacementMap = true;
		}

		buf = "";
		//matfile.getProperty(getKey(mname, "ambientMap"), buf);
		if (trim(buf) != "")
		{
			Path texpath(materialroot);
			texpath.addDirectories(buf);
			m_graphics->getTextureManager()->load(texpath, select.material.ambientMap, 0);
		}
		//*/

		m_selections.push_back(select);
		idx++;
	}

	tsinfo("materials imported successfully");

	SVertexMesh mesh;
	mesh.indexData = vector<Index>(&indices[0], &indices[0] + indices.size());
	mesh.vertexData = vector<byte>((const byte*)&vertices[0], (const byte*)&vertices[0] + (vertices.size() * sizeof(SModelVertex)));
	mesh.vertexTopology = EVertexTopology::eTopologyTriangleList;
	mesh.vertexStride = sizeof(SModelVertex);

	getAttributeList(mesh.vertexAttributes, modelAttribMask & attribMask);

	if (EMeshStatus status = m_graphics->getMeshManager()->createMesh(mesh, m_modelMesh))
	{
		tswarn("Unable to load mesh data: %", status);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#define VECTOR_OFFSET(idx) (uint32)(idx * sizeof(Vector))

void getAttributeList(vector<SVertexAttribute>& attribs, uint8 vertexFlags)
{
	//Position attribute
	if (vertexFlags & eModelVertexAttributePosition)
	{
		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = VECTOR_OFFSET(0);
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = "POSITION";
		sid.type = EVertexAttributeType::eAttribFloat4;
		attribs.push_back(sid);
	}

	//Texcoord attribute
	if (vertexFlags & eModelVertexAttributeTexcoord)
	{
		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = VECTOR_OFFSET(1);
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = "TEXCOORD";
		sid.type = EVertexAttributeType::eAttribFloat2;
		attribs.push_back(sid);
	}

	//Colour attribute
	if (vertexFlags & eModelVertexAttributeColour)
	{
		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = VECTOR_OFFSET(2);
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = "COLOUR";
		sid.type = EVertexAttributeType::eAttribFloat4;
		attribs.push_back(sid);
	}

	//Normal attribute
	if (vertexFlags & eModelVertexAttributeNormal)
	{
		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = VECTOR_OFFSET(3);
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = "NORMAL";
		sid.type = EVertexAttributeType::eAttribFloat3;
		attribs.push_back(sid);
	}

	//Tangent attribute
	if (vertexFlags & eModelVertexAttributeTangent)
	{
		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = VECTOR_OFFSET(4);
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = "TANGENT";
		sid.type = EVertexAttributeType::eAttribFloat3;
		attribs.push_back(sid);
	}

	//Bitangent attribute
	if (vertexFlags & eModelVertexAttributeBitangent)
	{
		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = VECTOR_OFFSET(5);
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = "BITANGENT";
		sid.type = EVertexAttributeType::eAttribFloat3;
		attribs.push_back(sid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////