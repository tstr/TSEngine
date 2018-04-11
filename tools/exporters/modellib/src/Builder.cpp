/*
    Model builder source
*/

#include "Builder.h"

#include <tsconfig.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

//Engine headers
#include <tscore/path.h>
#include <tscore/pathutil.h>
#include <tscore/system/time.h>
#include <tscore/debug/assert.h>

//Model format types and constants
#include <tsgraphics/model/modeldefs.h>
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

	modelfilepath.addDirectories(filename + ".tsm");
	matrlfilepath.addDirectories(filename + ".tmat");

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
	SModelHeader header;
	header.numMeshes = m_scene->mNumMeshes;
	
	vector<SModelMesh> meshes;
	vector<SModelVertex> vertices;
	vector<ModelIndex> indices;
	
	meshes.reserve(header.numMeshes);
	
	for (uint i = 0; i < header.numMeshes; i++)
	{
		SModelMesh mesh;
		
		aiMesh* aimesh = m_scene->mMeshes[i];
		
		aiString materialName;
		m_scene->mMaterials[aimesh->mMaterialIndex]->Get(AI_MATKEY_NAME, materialName);
		
		if (strlen(materialName.C_Str()) > MaxMaterialNameLength)
		{
			//warning material name too long
		}

		mesh.materialName.set(materialName.C_Str());
		
		//Vertex attribute mask
		uint8 attribs = 0;
		
		for (uint j = 0; j < aimesh->mNumVertices; j++)
		{
			SModelVertex vertex;

			if (aimesh->HasPositions())
			{
				vertex.position.x() = aimesh->mVertices[j].x;
				vertex.position.y() = aimesh->mVertices[j].y;
				vertex.position.z() = aimesh->mVertices[j].z;
				vertex.position.w() = 1.0f;

				attribs |= eModelVertexAttributePosition;
			}

			if (aimesh->HasNormals())
			{
				vertex.normal.x() = aimesh->mNormals[j].x;
				vertex.normal.y() = aimesh->mNormals[j].y;
				vertex.normal.z() = aimesh->mNormals[j].z;
				vertex.normal.normalize();

				attribs |= eModelVertexAttributeNormal;
			}

			if (aimesh->HasVertexColors(0))
			{
				vertex.colour.x() = aimesh->mColors[0][j].r;
				vertex.colour.y() = aimesh->mColors[0][j].g;
				vertex.colour.z() = aimesh->mColors[0][j].b;
				vertex.colour.w() = aimesh->mColors[0][j].a;

				attribs |= eModelVertexAttributeColour;
			}

			if (aimesh->HasTextureCoords(0))
			{
				vertex.texcoord.x() = aimesh->mTextureCoords[0][j].x;
				vertex.texcoord.y() = aimesh->mTextureCoords[0][j].y;

				attribs |= eModelVertexAttributeTexcoord;
			}

			if (aimesh->HasTangentsAndBitangents())
			{
				vertex.tangent.x() = aimesh->mTangents[j].x;
				vertex.tangent.y() = aimesh->mTangents[j].y;
				vertex.tangent.z() = aimesh->mTangents[j].z;
				vertex.tangent.normalize();

				attribs |= eModelVertexAttributeTangent;

				vertex.bitangent.x() = aimesh->mBitangents[j].x;
				vertex.bitangent.y() = aimesh->mBitangents[j].y;
				vertex.bitangent.z() = aimesh->mBitangents[j].z;
				vertex.bitangent.normalize();

				attribs |= eModelVertexAttributeBitangent;
			}
			
			vertices.push_back(vertex);
		}
		
		ModelIndex indexcount = 0;
		ModelIndex indexoffset = (ModelIndex)indices.size();
		
		for (UINT c = 0; c < aimesh->mNumFaces; c++)
		{
			for (UINT e = 0; e < aimesh->mFaces[c].mNumIndices; e++)
			{
				indices.push_back(aimesh->mFaces[c].mIndices[e]);
				indexcount++;
			}
		}
		
		mesh.indexOffset = indexoffset;
		mesh.indexCount = indexcount;
		mesh.numVertices = aimesh->mNumVertices;
		mesh.vertexAttributeMask = attribs;
		
		meshes.push_back(mesh);
	}
	
	header.numVertices = (ModelIndex)vertices.size();
	header.numIndices = (ModelIndex)indices.size();
	
	outputstream.write(reinterpret_cast<const char*>(&header), sizeof(SModelHeader));
	outputstream.write(reinterpret_cast<const char*>(&meshes[0]), sizeof(SModelMesh) * meshes.size());
	outputstream.write(reinterpret_cast<const char*>(&vertices[0]), sizeof(SModelVertex) * vertices.size());
	outputstream.write(reinterpret_cast<const char*>(&indices[0]), sizeof(ModelIndex) * indices.size());
	
	return outputstream.good();
}

inline void writeColourToStream(ostream& stream, const aiColor3D& colour)
{
	stream << colour.r << ", " << colour.g << ", " << colour.b;
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
			texpath.composePath(tex.C_Str());
			if ((string)texpath.str() != "")
				outputstream << "diffuseMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &tex);
			texpath.composePath(tex.C_Str());
			if ((string)texpath.str() != "")
				outputstream << "specularMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_AMBIENT, 0, &tex);
			texpath.composePath(tex.C_Str());
			if ((string)texpath.str() != "")
				outputstream << "ambientMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_DISPLACEMENT, 0, &tex);
			texpath.composePath(tex.C_Str());
			if ((string)texpath.str() != "")
				outputstream << "displacementMap = " << texpath.str() << endl;

			tex.Clear();
			aimaterial->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &tex);
			texpath.composePath(tex.C_Str());
			if ((string)texpath.str() != "")
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
