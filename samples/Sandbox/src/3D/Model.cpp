/*
	Model Importer class
*/


#include "Model.h"

#include <tsgraphics/schemas/Model.rcs.h>

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

void findAttribute(const char* semantic, EVertexAttributeType type, const unordered_map<string, uint32>& attribMap, vector<SVertexAttribute>& attribs);

/////////////////////////////////////////////////////////////////////////////////////////////////

CModel::CModel(GraphicsContext* graphics) :
	m_graphics(graphics)
{
	tsassert(m_graphics);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Import function
/////////////////////////////////////////////////////////////////////////////////////////////////

bool CModel::import(const Path& path)
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

	rc::ResourceLoader loader(modelfile);
	auto& modelReader = loader.deserialize<tsr::Model>();

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
	materialpathname += ".mat";
	Path materialpath(materialpathname);
	Path materialroot(materialpath.getParent());

	tsinfo("creating materials \"%\"...", materialpath.str());

	//INI parser
	INIReader matfile(materialpath.str());

	size_t idx = 0;

	int32 basevertex = 0;

	//Iterate over model meshes
	for (uint32 i = 0; i < modelReader.meshes().length(); i++)
	{
		const tsr::Mesh& fmesh = modelReader.meshes()[i];

		Selection select;
		
		select.submesh.indexOffset = fmesh.indexOffset();
		select.submesh.indexCount = fmesh.indexCount();
		select.submesh.vertexBase = basevertex;
		basevertex += fmesh.vertexCount();

		const char* mname = fmesh.materialName().str();

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
	mesh.indexData = modelReader.indexData().toVector();
	mesh.vertexData = modelReader.vertexData().toVector();
	mesh.vertexTopology = EVertexTopology::eTopologyTriangleList;
	mesh.vertexStride = modelReader.vertexStride();

	std::unordered_map<String, uint32> attributes;

	for (uint32 i = 0; i < modelReader.attributeNames().length(); i++)
	{
		attributes[modelReader.attributeNames()[i].stdStr()] = modelReader.attributeOffsets()[i];
	}

	for (auto x : { "POSITION","TEXCOORD0","COLOUR0" })
	{

	}

	findAttribute("POSITION",  EVertexAttributeType::eAttribFloat4, attributes, mesh.vertexAttributes);
	findAttribute("TEXCOORD0", EVertexAttributeType::eAttribFloat2, attributes, mesh.vertexAttributes);
	findAttribute("COLOUR0",   EVertexAttributeType::eAttribFloat4, attributes, mesh.vertexAttributes);
	findAttribute("NORMAL",    EVertexAttributeType::eAttribFloat3, attributes, mesh.vertexAttributes);
	findAttribute("TANGENT",   EVertexAttributeType::eAttribFloat3, attributes, mesh.vertexAttributes);
	findAttribute("BITANGENT", EVertexAttributeType::eAttribFloat3, attributes, mesh.vertexAttributes);

	if (EMeshStatus status = m_graphics->getMeshManager()->createMesh(mesh, m_modelMesh))
	{
		tswarn("Unable to load mesh data: %", status);
	}

	return true;
}

void findAttribute(const char* semantic, EVertexAttributeType type, const unordered_map<string, uint32>& attribMap, vector<SVertexAttribute>& attribs)
{
	auto it = attribMap.find(semantic);
	if (it != attribMap.end())
	{
		if ((string)semantic == "TEXCOORD0") semantic = "TEXCOORD";
		if ((string)semantic == "COLOUR0") semantic = "COLOUR";

		SVertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = it->second;
		sid.channel = EVertexAttributeChannel::eChannelPerVertex;
		sid.semanticName = semantic;
		sid.type = type;
		attribs.push_back(sid);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////