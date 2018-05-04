/*
    Model builder source
*/

#include "Builder.h"

#include <tsconfig.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

//Engine headers
#include <tscore/path.h>
#include <tscore/pathutil.h>
#include <tscore/system/time.h>
#include <tscore/debug/assert.h>

//Model format types and constants
#include <tsgraphics/schemas/Model.rcs.h>

//Assimp headers
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////

Model::Model()
{
    /*
        Parse list of file extensions supported by Assimp
    */
    std::string ext;
    std::string extList;
    m_imp.GetExtensionList(extList);

    auto it = extList.begin();

    //Parse ; separated list
    while (it != extList.end())
    {
        ext.clear();

        assert(*it == '*'); it++;
        assert(*it == '.'); it++;

        while (it != extList.end() && *it != ';')
        {
            ext += *it;
            it++;
        }

        if (it != extList.end() && *it == ';')
            it++;

        m_extensions.insert(ext);
    }

    /*
        Attach logger
    */
	attachAILogger(false);
}

bool Model::imp(const std::string& modelFileName)
{
	m_sceneFile = modelFileName;

	m_scene = m_imp.ReadFile(Path(m_sceneFile).str(),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_OptimizeMeshes |
		//aiProcess_GenNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_ConvertToLeftHanded |
		//aiProcess_SortByPType |
		aiProcess_PreTransformVertices
	);

    return m_scene != nullptr;
}

bool Model::exp(const std::string& outputDir)
{
	tsassert(m_scene != nullptr);

	Path targetpath(m_sceneFile);

	Path modelfilepath(outputDir);
	Path matrlfilepath(outputDir);

	//Extract filename from target
	string filename = targetpath.getDirectoryTop().str();
	filename.erase(filename.find_last_of('.'), string::npos);

	modelfilepath.addDirectories(filename + ".model");
	matrlfilepath.addDirectories(filename + ".mat");

	//Open model file stream
    fstream modelfile(createFile(modelfilepath, ios::binary | ios::out));
	fstream matfile(createFile(matrlfilepath, ios::binary | ios::out));
	
	//Export Assimp model data to custom format
    if (exportModel(modelfile) && exportMaterials(matfile))
	{
		return true;
	}
	
	return false;
}

bool Model::exportModel(std::ostream& outputstream)
{
	tsr::ModelBuilder modelWriter;

	vector<rc::Ref<tsr::Mesh>> meshes;
	vector<float> vertices; //vertex buffer
	vector<uint32> indices; //index buffer
	unordered_map<string, uint32> attributes;
	uint32 vertexStride = 0;
	
	meshes.reserve(m_scene->mNumMeshes);

	//Foreach mesh
	for (uint i = 0; i < m_scene->mNumMeshes; i++)
	{
		aiMesh* aimesh = m_scene->mMeshes[i];
		
		aiString materialName;
		m_scene->mMaterials[aimesh->mMaterialIndex]->Get(AI_MATKEY_NAME, materialName);

		tsr::MeshBuilder mesh(modelWriter);
		
		for (uint j = 0; j < aimesh->mNumVertices; j++)
		{
			uint32 attributeOffset = 0;

			if (aimesh->HasPositions())
			{
				vertices.push_back(aimesh->mVertices[j].x);
				vertices.push_back(aimesh->mVertices[j].y);
				vertices.push_back(aimesh->mVertices[j].z);
				vertices.push_back(1.0f);

				attributes["POSITION"] = attributeOffset;
				attributeOffset += sizeof(float[4]);
			}

			if (aimesh->HasNormals())
			{
				vertices.push_back(aimesh->mNormals[j].x);
				vertices.push_back(aimesh->mNormals[j].y);
				vertices.push_back(aimesh->mNormals[j].z);
				vertices.push_back(0.0f);

				attributes["NORMAL"] = attributeOffset;
				attributeOffset += sizeof(float[4]);
			}

			for (uint32 c = 0; aimesh->HasVertexColors(c); c++)
			{
				vertices.push_back(aimesh->mColors[c][j].r);
				vertices.push_back(aimesh->mColors[c][j].g);
				vertices.push_back(aimesh->mColors[c][j].b);
				vertices.push_back(aimesh->mColors[c][j].a);

				attributes[(string)"COLOUR" + to_string(c)] = attributeOffset;
				attributeOffset += sizeof(float[4]);
			}

			for (uint32 t = 0; aimesh->HasTextureCoords(t); t++)
			{
				vertices.push_back(aimesh->mTextureCoords[t][j].x);
				vertices.push_back(aimesh->mTextureCoords[t][j].y);
				vertices.push_back(aimesh->mTextureCoords[t][j].z);
				vertices.push_back(0.0f);

				attributes[(string)"TEXCOORD" + to_string(t)] = attributeOffset;
				attributeOffset += sizeof(float[4]);
			}

			if (aimesh->HasTangentsAndBitangents())
			{
				vertices.push_back(aimesh->mTangents[j].x);
				vertices.push_back(aimesh->mTangents[j].y);
				vertices.push_back(aimesh->mTangents[j].z);
				vertices.push_back(0.0f);

				attributes["TANGENT"] = attributeOffset;
				attributeOffset += sizeof(float[4]);

				vertices.push_back(aimesh->mBitangents[j].x);
				vertices.push_back(aimesh->mBitangents[j].y);
				vertices.push_back(aimesh->mBitangents[j].z);
				vertices.push_back(0.0f);

				attributes["BITANGENT"] = attributeOffset;
				attributeOffset += sizeof(float[4]);
			}

			vertexStride = attributeOffset;
		}
		
		uint32 indexcount = 0;
		uint32 indexoffset = (uint32)indices.size();
		
		for (UINT c = 0; c < aimesh->mNumFaces; c++)
		{
			for (UINT e = 0; e < aimesh->mFaces[c].mNumIndices; e++)
			{
				indices.push_back(aimesh->mFaces[c].mIndices[e]);
				indexcount++;
			}
		}
		
		mesh.set_indexOffset(indexoffset);
		mesh.set_indexCount(indexcount);
		mesh.set_vertexCount(aimesh->mNumVertices);
		mesh.set_vertexBase(0);
		mesh.set_materialName(mesh.createString(materialName.C_Str()));

		meshes.push_back(mesh.build());
	}

	modelWriter.set_indexData(modelWriter.createArray(indices));
	modelWriter.set_vertexData((rc::Ref<rc::ArrayView<ts::byte>>)modelWriter.createArray(vertices));
	modelWriter.set_meshes(modelWriter.createArrayOfRefs(meshes));

	modelWriter.set_vertexStride(vertexStride);

	vector<string> attributeNames;
	vector<uint32> attributeOffsets;

	for (const auto& entry : attributes)
	{
		attributeNames.push_back(entry.first);
		attributeOffsets.push_back(entry.second);
	}

	modelWriter.set_attributeNames(modelWriter.createArrayOfStrings(attributeNames));
	modelWriter.set_attributeOffsets(modelWriter.createArray(attributeOffsets));


	modelWriter.build(outputstream);

	return outputstream.good();
}

inline void writeColourToStream(ostream& stream, const aiColor3D& colour)
{
	stream << colour.r << ", " << colour.g << ", " << colour.b;
}

bool formatTexPath(const aiString& texName, Path& texPath)
{
	if (texName.length == 0)
		return false;

	string name = texName.C_Str();
	name = name.substr(0, name.find_last_of('.')).append(".texture");

	std::cout << name << std::endl;

	texPath.composePath(name);

	return true;
}

bool Model::exportMaterials(std::ostream& outputstream)
{
	outputstream << "#\n";
	outputstream << "#	Material file\n";
	outputstream << "#\n\n";

	if (m_scene->HasMaterials())// || !no_materials)
	{
		for (uint32 i = 0; i < m_scene->mNumMaterials; i++)
		{
			aiMaterial* aimaterial = m_scene->mMaterials[i];
			
			aiString aimatName;
			aimaterial->Get(AI_MATKEY_NAME, aimatName);
			
			outputstream << "[" << aimatName.C_Str() << "]\n";
			
			//Set colour properties
			aiColor3D colour;

			aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
			outputstream << "diffuseColour = ";
			writeColourToStream(outputstream, colour);
			outputstream << "\n";

			aimaterial->Get(AI_MATKEY_COLOR_AMBIENT, colour);
			outputstream << "ambientColour = ";
			writeColourToStream(outputstream, colour);
			outputstream << "\n";
			
			aimaterial->Get(AI_MATKEY_COLOR_EMISSIVE, colour);
			outputstream << "emissiveColour = ";
			writeColourToStream(outputstream, colour);
			outputstream << "\n";
			
			aimaterial->Get(AI_MATKEY_COLOR_SPECULAR, colour);
			outputstream << "specularColour = ";
			writeColourToStream(outputstream, colour);
			outputstream << "\n";
			
			float shininess = 0.0f;
			float alpha = 0.0f;
			aimaterial->Get(AI_MATKEY_SHININESS, shininess);
			outputstream << "shininess = " << shininess << "\n";
			aimaterial->Get(AI_MATKEY_OPACITY, alpha);
			outputstream << "alpha = " << alpha << "\n";
			
			//Set texture path properties
			aiString tex;
			Path texpath;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &tex);
			if (formatTexPath(tex, texpath))
				outputstream << "diffuseMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &tex);
			if (formatTexPath(tex, texpath))
				outputstream << "specularMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_AMBIENT, 0, &tex);
			if (formatTexPath(tex, texpath))
				outputstream << "ambientMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_DISPLACEMENT, 0, &tex);
			if (formatTexPath(tex, texpath))
				outputstream << "displacementMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &tex);
			if (formatTexPath(tex, texpath))
				outputstream << "normalMap = " << texpath.str() << endl;

			outputstream << endl;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////

void Model::attachAILogger(bool verbose)
{
	Assimp::Logger::LogSeverity severity = (verbose) ? Assimp::Logger::VERBOSE : Assimp::Logger::NORMAL;

	// Create a logger instance for Console Output
	Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
}

///////////////////////////////////////////////////////////////////////////////////////////////
