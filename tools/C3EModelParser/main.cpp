/*
	Model parser tool
*/

#include "pch.h"

#include <C3E\gfx\graphicsmodel.h>
#include <C3E\core\time.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include "assimp\Importer.hpp"
#include "assimp\material.h"
#include "assimp\mesh.h"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

LINK_LIB("C3EGraphics.lib")
LINK_LIB("assimp.lib")

using namespace std;
using namespace C3E;

enum ModelLoadFlags
{
	MODEL_FLAG_NOMAT = 1
};

///////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		cout << "No valid file specified\n";
		return EXIT_FAILURE;
	}

	const char* path = argv[1];	

	bool no_materials = false;
	if (argv[2])
	{
		no_materials = compare_string_weak(argv[2], "nomat");
	}

	if (!path)
	{
		cout << "No valid file specified\n";
		return EXIT_FAILURE;
	}

	bool success = false;
	int flags = (no_materials) ? MODEL_FLAG_NOMAT : 0;

	Assimp::Importer imp;

	cout << "File parsing...\n";

	Stopwatch t;
	t.Start();

	const aiScene* scene = imp.ReadFile(path, 
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

	t.Stop();
	double dt = t.DeltaTime();

	if (scene)
	{
		cout << "File parsed successfully.\n\n";

		ModelExporter exporter;

		uint32 num_meshes = scene->mNumMeshes;
		uint32 num_materials = scene->mNumMaterials;

		vector<Vertex> vertices;
		vector<Index> indices;

		for (uint32 i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];

			vertices.clear();
			indices.clear();

			aiString mName;
			scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, mName);

			const char* matName = mName.C_Str();
			const char* meshName = mesh->mName.C_Str();

			uint32 vertexAttributeMask = 0;

			for (UINT j = 0; j < mesh->mNumVertices; j++)
			{
				Vertex vertex;

				Vector position;
				Vector texcoord;
				Vector normal;
				Vector colour;
				Vector tangent;
				Vector bitangent;

				if (mesh->HasPositions())
				{
					position.x() = mesh->mVertices[j].x;
					position.y() = mesh->mVertices[j].y;
					position.z() = mesh->mVertices[j].z;
					position.w() = 1.0f;
					vertex.set((uint32)VertexAttributeIndex::Position, position);

					vertexAttributeMask |= (uint32)VertexAttributes::Position;
				}

				if (mesh->HasNormals())
				{
					normal.x() = mesh->mNormals[j].x;
					normal.y() = mesh->mNormals[j].y;
					normal.z() = mesh->mNormals[j].z;
					normal.Normalize();
					vertex.set((uint32)VertexAttributeIndex::Normal, normal);

					vertexAttributeMask |= (uint32)VertexAttributes::Normal;
				}

				if (mesh->HasVertexColors(0))
				{
					colour.x() = mesh->mColors[0][j].r;
					colour.y() = mesh->mColors[0][j].g;
					colour.z() = mesh->mColors[0][j].b;
					colour.w() = mesh->mColors[0][j].a;
					vertex.set((uint32)VertexAttributeIndex::Colour, colour);

					vertexAttributeMask |= (uint32)VertexAttributes::Colour;
				}

				if (mesh->HasTextureCoords(0))
				{
					texcoord.x() = mesh->mTextureCoords[0][j].x;
					texcoord.y() = mesh->mTextureCoords[0][j].y;
					vertex.set((uint32)VertexAttributeIndex::Texcoord, texcoord);

					vertexAttributeMask |= (uint32)VertexAttributes::Texcoord;
				}

				if (mesh->HasTangentsAndBitangents())
				{
					tangent.x() = mesh->mTangents[j].x;
					tangent.y() = mesh->mTangents[j].y;
					tangent.z() = mesh->mTangents[j].z;
					tangent.Normalize();
					vertex.set((uint32)VertexAttributeIndex::Tangent, tangent);

					vertexAttributeMask |= (uint32)VertexAttributes::Tangent;


					bitangent.x() = mesh->mBitangents[j].x;
					bitangent.y() = mesh->mBitangents[j].y;
					bitangent.z() = mesh->mBitangents[j].z;
					bitangent.Normalize();
					vertex.set((uint32)VertexAttributeIndex::Bitangent, bitangent);

					vertexAttributeMask |= (uint32)VertexAttributes::Bitangent;
				}

				vertices.push_back(vertex);
			}

			for (UINT c = 0; c < mesh->mNumFaces; c++)
			{
				for (UINT e = 0; e < mesh->mFaces[c].mNumIndices; e++)
				{
					indices.push_back(mesh->mFaces[c].mIndices[e]);
				}
			}

			exporter.AddMesh(meshName, vertices, indices, vertexAttributeMask, matName);
		}
		
		if (scene->HasMaterials())// || !no_materials)
		{
			for (uint32 i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* material = scene->mMaterials[i];

				ModelMaterial fmat;

				aiString matName;
				material->Get(AI_MATKEY_NAME, matName);

				strcpy_s(fmat.name, matName.C_Str());

				cout << "Loading material: " << fmat.name << endl;

				aiColor3D colour;

				material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
				fmat.cDiffuse.x() = colour.r;
				fmat.cDiffuse.y() = colour.g;
				fmat.cDiffuse.z() = colour.b;
				fmat.cDiffuse.w() = 1.0f;

				material->Get(AI_MATKEY_COLOR_AMBIENT, colour);
				fmat.cAmbient.x() = colour.r;
				fmat.cAmbient.y() = colour.g;
				fmat.cAmbient.z() = colour.b;
				fmat.cAmbient.w() = 1.0f;

				material->Get(AI_MATKEY_COLOR_EMISSIVE, colour);
				fmat.cEmissive.x() = colour.r;
				fmat.cEmissive.y() = colour.g;
				fmat.cEmissive.z() = colour.b;
				fmat.cEmissive.w() = 1.0f;

				material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
				fmat.cSpecular.x() = colour.r;
				fmat.cSpecular.y() = colour.g;
				fmat.cSpecular.z() = colour.b;
				fmat.cSpecular.w() = 1.0f;

				material->Get(AI_MATKEY_SHININESS, fmat.shininess);
				material->Get(AI_MATKEY_OPACITY, fmat.alpha);
				

				aiString tex;

				tex.Clear();
				material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &tex);
				cout << "Diffuse map = " << tex.C_Str() << endl;
				strcpy_s(fmat.texDiffuse, tex.C_Str());

				tex.Clear();
				material->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &tex);
				cout << "Specular map = " << tex.C_Str() << endl;
				strcpy_s(fmat.texSpecular, tex.C_Str());

				tex.Clear();
				material->GetTexture(aiTextureType::aiTextureType_AMBIENT, 0, &tex);
				cout << "Ambient map = " << tex.C_Str() << endl;
				strcpy_s(fmat.texAmbient, tex.C_Str());

				tex.Clear();
				material->GetTexture(aiTextureType::aiTextureType_DISPLACEMENT, 0, &tex);
				cout << "Displacement map = " << tex.C_Str() << endl;
				strcpy_s(fmat.texDisplacement, tex.C_Str());

				tex.Clear();
				material->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &tex);
				cout << "Normal map = " << tex.C_Str() << endl;
				strcpy_s(fmat.texNormal, tex.C_Str());

				exporter.AddMaterial(fmat);
			}
		}

		uint32 num_indices = (uint32)indices.size();
		uint32 num_vertices = (uint32)vertices.size();

		string fname(path);
		size_t pos = fname.find_last_of('.');
		if (pos == string::npos)
		{
			cerr << "File name could not be parsed\n";
			return EXIT_FAILURE;
		}
		fname = fname.substr(0, pos);
		fname += ".";
		fname += g_model_extension;

		exporter.SetScale(1.0f);
		if (!exporter.Save(fname.c_str()))
			return EXIT_FAILURE;

		cout << "\nModel file generated successfully: " << fname << "\n";
		cout << "Elapsed time = " << fixed << dt << "ms\n";
		cout << "Num vertice = " << num_vertices << endl;
		cout << "Num indices = " << num_indices << endl;
		cout << "Num materials = " << num_materials << endl;
		cout << "Num meshes = " << num_meshes << endl;

		//cout << "Vertex attribute mask = " << header.vertexAttributeMask << endl;
	}
	else
	{
		cout << "Failed to parse file.\n";
		cout << imp.GetErrorString() << endl;
		return EXIT_FAILURE;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//OBJ Model loader
///////////////////////////////////////////////////////////////////////////////////////////////

/*
bool LoadOBJModel(const char* path, int flags)
{
	ModelOBJ model;

	Stopwatch t;
	t.Start();
	bool success = model.import(path, false);
	//model.reverseWinding();
	t.Stop();
	double dt = t.DeltaTime();

	if (success)
	{
		const ModelOBJ::Vertex* vertices = model.getVertexBuffer();
		const int* indices = model.getIndexBuffer();

		string matfilename = GetFilename(path);
		size_t pos = matfilename.find_last_of('.');

		if (pos != string::npos)
		{
			matfilename.erase(pos, matfilename.size() - pos);
		}

		matfilename += (string)"." + g_material_extension;

		string matpath(model.getPath() + matfilename);
		MaterialLoader matloader(matpath.c_str());

		ptrdiff_t buffer_offset = 0;
		ptrdiff_t buffer_size =
			sizeof(ModelFileHeader) +
			(sizeof(ModelFileMesh) * model.getNumberOfMeshes()) +
			(sizeof(Vertex) * model.getNumberOfVertices()) +
			(sizeof(Index) * model.getNumberOfIndices());

		void* buffer = malloc(buffer_size);

		///////////////////////////////////////////////////////////////////////////////////////////////
		//Write file header

		ModelFileHeader header;
		header.num_vertices = model.getNumberOfVertices();
		header.num_indices = model.getNumberOfIndices();
		header.num_meshes = model.getNumberOfMeshes();

		strcpy_s(header.materialLibrary, matfilename.c_str());

		model.getCenter(
			header.center.x(),
			header.center.y(),
			header.center.z()
			);

		header.radius = model.getRadius();

		if (model.hasPositions()) header.vertexAttributeMask |= (uint32)VertexAttributes::Position;
		if (model.hasNormals()) header.vertexAttributeMask |= (uint32)VertexAttributes::Normal;
		if (model.hasTextureCoords()) header.vertexAttributeMask |= (uint32)VertexAttributes::Texcoord;
		if (model.hasTangents()) header.vertexAttributeMask |= (uint32)VertexAttributes::Tangent;

		memcpy((void*)((ptrdiff_t)buffer + buffer_offset), &header, sizeof(ModelFileHeader));
		buffer_offset += sizeof(ModelFileHeader);

		///////////////////////////////////////////////////////////////////////////////////////////////
		//Write meshes and materials

		for (int x = 0; x < model.getNumberOfMeshes(); x++)
		{
			auto& cmesh = model.getMesh(x);
			auto& cmat = *cmesh.pMaterial;

			ModelFileMesh mesh;
			MaterialLoader::Material mat;

			strcpy_s(mesh.materialName, cmat.name.c_str());
			mesh.startIndex = cmesh.startIndex;
			mesh.vertexCount = cmesh.triangleCount * 3;

			if (!(flags & MODEL_FLAG_NOMAT))
			{
				mat.name = cmat.name;
				mat.alpha = cmat.alpha;
				mat.shininess = cmat.shininess;

				mat.ambient = Vector(cmat.ambient);
				mat.diffuse = Vector(cmat.diffuse);
				mat.specular = Vector(cmat.specular);

				mat.textures[0] = cmat.colorMapFilename;
				mat.textures[1] = cmat.bumpMapFilename;

				matloader.WriteMaterial(mat);
			}

			memcpy((void*)((ptrdiff_t)buffer + buffer_offset), &mesh, sizeof(ModelFileMesh));
			buffer_offset += sizeof(ModelFileMesh);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		//Write vertices

		vector<Vertex> copy_vertices(model.getNumberOfVertices());
		int vcount = model.getNumberOfVertices();
		Vector* tan1 = new Vector[vcount * 2];
		Vector* tan2 = tan1 + vcount;
		ZeroMemory(tan1, vcount * sizeof(Vector) * 2);

		for (int x = 0; x < model.getNumberOfVertices(); x++)
		{
			Vertex v;

			memcpy(&v.get((uint32)VertexAttributeIndex::Position), vertices[x].position, sizeof(float[3]));

			memcpy(&v.get((uint32)VertexAttributeIndex::Texcoord), vertices[x].texCoord, sizeof(float[2]));

			Vector& tex = v.get((uint32)VertexAttributeIndex::Texcoord);
			tex.y() = 1.0f - tex.y(); //invert V coordinate

			Vector tangent;
			Vector bitangent;
			Vector normal;

			memcpy(&normal, vertices[x].normal, sizeof(float[3]));
			v.set((uint32)VertexAttributeIndex::Normal, normal);

			memcpy(&tangent, vertices[x].tangent, sizeof(float[4]));
			v.set((uint32)VertexAttributeIndex::Tangent, tangent);

			//memcpy(&bitangent, vertices[x].bitangent, sizeof(float[3]));
			bitangent = Vector::Cross(normal, tangent);
			v.set((uint32)VertexAttributeIndex::Bitangent, bitangent);

			memcpy((void*)((ptrdiff_t)buffer + buffer_offset), &v, sizeof(Vertex));
			buffer_offset += sizeof(Vertex);
		}

		//Copy indices
		for (int x = 0; x < model.getNumberOfIndices(); x++)
		{
			Index i = (const Index&)(indices[x]);

			memcpy((void*)((ptrdiff_t)buffer + buffer_offset), &i, sizeof(Index));
			buffer_offset += sizeof(Index);
		}

		///////////////////////////////////////////////////////////////////////////////////////////////

		//Save file
		auto f(split(GetFilename(path), '.')[0] + "." + g_model_extension);
		f = model.getPath() + f;

		ofstream file(f, ios::binary);
		file.write((char*)buffer, buffer_size);
		file.close();

		///////////////////////////////////////////////////////////////////////////////////////////////

		free(buffer);

		cout << "Elapsed time: " << fixed << dt << "ms\n";
		cout << "Format = OBJ\n";
		cout << "Num vertices = " << model.getNumberOfVertices() << endl;
		cout << "Num indices = " << model.getNumberOfIndices() << endl;
		cout << "Num materials = " << model.getNumberOfMaterials() << endl;
		cout << "Num meshes = " << model.getNumberOfMeshes() << endl;

		cout << "Vertex attribute mask = " << header.vertexAttributeMask << endl;
	}
	else
	{
		return false;
	}

	return true;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////