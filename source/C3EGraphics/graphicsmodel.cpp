/*
	Graphics model loading API source
*/

#include "pch.h"

#include <C3E\gfx\graphicsmodel.h>
#include <C3E\core\maths.h>

#include <C3E\filesystem\filecore.h>
#include <C3E\filesystem\filexml.h>

using namespace C3E;
using namespace std;


struct ModelFileHeader
{
	uint32 num_vertices = 0;
	uint32 num_indices = 0;
	uint32 num_meshes = 0;
	uint32 num_materials = 0;

	float scale = 1.0f;
	BoundingBox bounds;
};

static void ComputeBoundingBox(const Vertex* vptr, uint32 size, BoundingBox& bb);
static void ComputeBoundingBoxIndexed(const Vertex* vptr, const Index*, uint32 size, BoundingBox& bb);

static bool ExportMaterials(const char* file, const vector<ModelMaterial>& materials);
static bool ImportMaterials(const char* file, vector<ModelMaterial>& materials);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ModelExporter::Impl
{
	vector<Vertex> m_vertices;
	vector<Index> m_indices;

	vector<ModelMesh> m_meshes;
	vector<ModelMaterial> m_materials;

	float m_scale = 1.0f;
};

struct ModelImporter::Impl
{
	string m_directory;
	
	vector<Vertex> m_vertices;
	vector<Index> m_indices;

	vector<ModelMesh> m_meshes;
	vector<ModelMaterial> m_materials;

	ModelFileHeader m_header;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//IMPORT MODEL
ModelImporter::ModelImporter(const char* filepath) :
	pImpl(new Impl)
{
	ifstream file(filepath, ios::binary);
	
	ModelFileHeader header;

	string modfilepath(filepath);
	size_t mpos = modfilepath.find_last_of('.');
	string matfilepath = modfilepath.substr(0, mpos);
	matfilepath += ".";
	matfilepath += g_material_extension;

	if (file)
	{
		file.read((char*)&header, sizeof(ModelFileHeader));

		if (file.fail())
		{
			cerr << "File header corrupted, unable to read model file: " << filepath << endl;
			return;
		}

		pImpl->m_materials.resize(header.num_materials);
		pImpl->m_meshes.resize(header.num_meshes);
		pImpl->m_vertices.resize(header.num_vertices);
		pImpl->m_indices.resize(header.num_indices);

		//file.read((char*)&pImpl->m_materials[0], sizeof(ModelMaterial) * header.num_materials);
		file.read((char*)&pImpl->m_meshes[0], sizeof(ModelMesh) * header.num_meshes);
		file.read((char*)&pImpl->m_vertices[0], sizeof(Vertex) * header.num_vertices);
		file.read((char*)&pImpl->m_indices[0], sizeof(Index) * header.num_indices);

		if (file.fail())
		{
			cerr << "Unable to read model file: " << filepath << endl;
			return;
		}

		if (!ImportMaterials(matfilepath.c_str(), pImpl->m_materials))
		{
			cerr << "Materials could not be imported: " << matfilepath << endl;
		}
	}
	else
	{
		cerr << "File stream could not be opened: " << filepath << endl;
		return;
	}

	C3E_ASSERT(file.good());

	pImpl->m_header = header;

	string str(filepath);
	size_t pos = str.find_last_of("\\");
	if (pos != string::npos)
	{
		pImpl->m_directory = str.substr(0, pos + 1);
	}
}

//EXPORT MODEL
bool ModelExporter::Save(const char* filepath)
{
	ofstream file(filepath, ios::binary | ios::trunc);

	ModelFileHeader header;

	string modfilepath(filepath);
	auto pos = modfilepath.find_last_of('.');
	string matfilepath = modfilepath.substr(0, pos);
	matfilepath += ".";
	matfilepath += g_material_extension;

	ComputeBoundingBoxIndexed(&pImpl->m_vertices[0], &pImpl->m_indices[0], (uint32)pImpl->m_indices.size(), header.bounds);

	header.num_indices = (uint32)pImpl->m_indices.size();
	header.num_vertices = (uint32)pImpl->m_vertices.size();
	header.num_materials = (uint32)pImpl->m_materials.size();
	header.num_meshes = (uint32)pImpl->m_meshes.size();
	header.scale = pImpl->m_scale;

	if (file)
	{
		//header -> materials -> meshes -> vertices -> indices
		file.write(reinterpret_cast<char*>(&header), sizeof(ModelFileHeader));
		//file.write(reinterpret_cast<char*>(&pImpl->m_materials[0]), sizeof(ModelMaterial) * header.num_materials);
		file.write(reinterpret_cast<char*>(&pImpl->m_meshes[0]), sizeof(ModelMesh) * header.num_meshes);
		file.write(reinterpret_cast<char*>(&pImpl->m_vertices[0]), sizeof(Vertex) * header.num_vertices);
		file.write(reinterpret_cast<char*>(&pImpl->m_indices[0]), sizeof(Index) * header.num_indices);

		if (file.fail())
		{
			cerr << "File stream could not be written\n";
			return false;
		}

		if (!ExportMaterials(matfilepath.c_str(), pImpl->m_materials))
		{
			cerr << "Materials could not be exported: " << matfilepath << endl;
			return false;
		}
	}
	else
	{
		cerr << "File stream could not be opened: " << filepath << endl;
		return false;
	}

	return true;
}

ModelImporter::~ModelImporter()
{
	if (pImpl)
		delete pImpl;
}

ModelExporter::ModelExporter() :
	pImpl(new Impl)
{}

ModelExporter::~ModelExporter()
{
	if (pImpl)
		delete pImpl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ModelImporter::GetMaterial(const char* name, ModelMaterial& mat) const
{
	for (const ModelMaterial& _mat : pImpl->m_materials)
	{
		if (compare_string_weak(_mat.name, name))
		{
			mat = _mat;
			return true;
		}
	}

	return false;
}
bool ModelImporter::GetMaterial(uint32 index, ModelMaterial& mat) const
{
	if (index > pImpl->m_materials.size()) return false;
	mat = pImpl->m_materials[index];
	return true;
}
bool ModelImporter::GetMesh(const char* name, ModelMesh& mesh) const
{
	for (auto& _mesh : pImpl->m_meshes)
	{
		if (strcmp(_mesh.nameMesh, name))
		{
			mesh = _mesh;
			return true;
		}
	}

	return false;
}
bool ModelImporter::GetMesh(uint32 index, ModelMesh& mesh) const
{
	if (index > pImpl->m_meshes.size()) return false;
	mesh = pImpl->m_meshes[index];
	return true;
}

uint32 ModelImporter::GetMeshCount() const
{
	return (uint32)pImpl->m_meshes.size();
}
uint32 ModelImporter::GetMaterialCount() const
{
	return (uint32)pImpl->m_materials.size();
}

const Vertex* ModelImporter::GetVertices() const
{
	return &pImpl->m_vertices[0];
}
const Index* ModelImporter::GetIndices() const
{
	return &pImpl->m_indices[0];
}
uint32 ModelImporter::GetIndexCount() const
{
	return (uint32)pImpl->m_indices.size();
}
uint32 ModelImporter::GetVertexCount() const
{
	return (uint32)pImpl->m_vertices.size();
}

void ModelImporter::GetBoundingBox(BoundingBox& box) const
{
	box = pImpl->m_header.bounds;
}

std::string ModelImporter::GetDirectory() const
{
	return pImpl->m_directory;
}

float ModelImporter::GetScale() const
{
	return pImpl->m_header.scale;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporter::AddMesh(
	const char* name,
	const Vertex* vertices,
	const Index* indices,
	uint32 vnum,
	uint32 inum,
	uint32 attributeMask,
	const char* material
	)
{
	ModelMesh mesh;
	
	ComputeBoundingBoxIndexed(vertices, indices, inum, mesh.bounds);

	//Incremement indices in current mesh by size of the vertex buffer
	uint32 vertex_offset = (uint32)pImpl->m_vertices.size();

	mesh.indexCount = inum;
	mesh.indexStart = (uint32)pImpl->m_indices.size();
	mesh.vertexAttributeMask = attributeMask;
	
	strcpy_s(mesh.nameMesh, name);
	strcpy_s(mesh.nameMaterial, material);

	for (uint32 i = 0; i < vnum; i++)
	{
		pImpl->m_vertices.push_back(vertices[i]);
	}

	for (uint32 j = 0; j < inum; j++)
	{
		pImpl->m_indices.push_back(indices[j] + vertex_offset);
	}

	pImpl->m_meshes.push_back(mesh);
}

void ModelExporter::AddMaterial(const ModelMaterial& mat)
{
	pImpl->m_materials.push_back(mat);
}

void ModelExporter::SetScale(float scale) { pImpl->m_scale = scale; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
struct MaterialLoader::Impl
{
	FileSystem fs;
	unique_ptr<FileSystem::IXMLfile> file;

	FileSystem::IXMLnode* materialNode = nullptr;

	bool modified = false;

	vector<Material> m_materials;

	Impl(const char* matlib)
	{
		bool write = false;
		modified = true;

		if (!FileSystem::FileExists(matlib))
		{
			ofstream f(matlib, ios::trunc);
			write = true;
		}

		file = fs.OpenXMLfile(matlib);

		cout << "Material file generated: " << matlib << endl;

		if (write)
		{
			auto declaration = file->AllocateNode(FileSystem::xml_declaration);
			declaration->AddAttribute("version", "1.0");
			declaration->AddAttribute("encoding", "UTF-8");
			file->Root()->AddNode(declaration);

			modified = true;
		}

		if (!(materialNode = file->Root()->GetNode("Materials")))
		{
			auto node = file->AllocateNode("Materials");
			file->Root()->AddNode(node);
			materialNode = node;

			modified = true;
		}
	}

	~Impl()
	{
		if (modified)
		{
			file->Save();
		}
	}
};

MaterialLoader::MaterialLoader(const char* matlib) :
	pImpl(new Impl(matlib))
{

}

MaterialLoader::~MaterialLoader()
{
	if (pImpl)
		delete pImpl;
}

bool MaterialLoader::ReadMaterial(const char* name, Material& mat)
{
	auto node_mat = pImpl->materialNode->GetNode(name);
	
	if (!node_mat)
		return false;
	
	mat.name = name;

	auto val = node_mat->GetAttribute("effect").value;
	if (!val) return false;
	mat.effect = val;

	//Material properties node
	auto node_props = node_mat->GetNode("Properties");

	if (node_props)
	{
		//Ambient colour
		{
			const char* val = nullptr;
			auto node = node_props->GetNode("ambient");

			val = node->GetAttribute("r").value;
			if (!val) return false;
			mat.ambient.x() = stof(val);

			val = node->GetAttribute("g").value;
			if (!val) return false;
			mat.ambient.y() = stof(val);

			val = node->GetAttribute("b").value;
			if (!val) return false;
			mat.ambient.z() = stof(val);

			val = node->GetAttribute("a").value;
			if (!val) return false;
			mat.ambient.w() = stof(val);
		}

		//Diffuse colour
		{
			const char* val = nullptr;
			auto node = node_props->GetNode("diffuse");

			val = node->GetAttribute("r").value;
			if (!val) return false;
			mat.diffuse.x() = stof(val);
			
			val = node->GetAttribute("g").value;
			if (!val) return false;
			mat.diffuse.y() = stof(val);

			val = node->GetAttribute("b").value;
			if (!val) return false;
			mat.diffuse.z() = stof(val);

			val = node->GetAttribute("a").value;
			if (!val) return false;
			mat.diffuse.w() = stof(val);
		}

		//Specular colour
		{
			const char* val = nullptr;
			auto node = node_props->GetNode("specular");

			val = node->GetAttribute("r").value;
			if (!val) return false;
			mat.specular.x() = stof(val);

			val = node->GetAttribute("g").value;
			if (!val) return false;
			mat.specular.y() = stof(val);

			val = node->GetAttribute("b").value;
			if (!val) return false;
			mat.specular.z() = stof(val);

			val = node->GetAttribute("a").value;
			if (!val) return false;
			mat.specular.w() = stof(val);
		}

		//Shininess
		{
			const char* val = nullptr;
			auto node = node_props->GetNode("shininess");
			val = node->Value();
			if (!val) return false;
			mat.shininess = stof(val);
		}

		//Alpha
		{
			const char* val = nullptr;
			auto node = node_props->GetNode("alpha");
			val = node->Value();
			if (!val) return false;
			mat.alpha = stof(val);
		}
	}

	auto node_textures = node_mat->GetNode("Textures");

	if (node_textures)
	{
		size_t i = 0;

		for (auto n = node_textures->begin(); n != node_textures->end(); n++)
		{
			if ((i + 1) >= mat.textures.size())
				break;

			size_t index = i;

			if (auto val = n->GetAttribute("index").value)
			{
				index = stol(val);
			}

			mat.textures[index] = (string)n->GetAttribute("file").value;

			i++;
		}
	}

	return true;
}

bool MaterialLoader::WriteMaterial(const Material& mat)
{
	if (mat.name == "") return false;

	auto file = pImpl->file.get();

	bool add_node = false;
	auto matnode = pImpl->materialNode->GetNode(mat.name.c_str());

	if (matnode)
	{
		matnode->DestroyAllNodes();
	}
	else
	{
		matnode = file->AllocateNode(mat.name.c_str());
		add_node = true;
	}

	//Material properties node
	auto node_props = file->AllocateNode("Properties");

	//Ambient colour
	{
		auto node = file->AllocateNode("ambient");

		node->AddAttribute("r", tocstr(mat.ambient.x()));
		node->AddAttribute("g", tocstr(mat.ambient.y()));
		node->AddAttribute("b", tocstr(mat.ambient.z()));
		node->AddAttribute("a", tocstr(mat.ambient.w()));

		node_props->AddNode(node);
	}

	//Diffuse colour
	{
		auto node = file->AllocateNode("diffuse");

		node->AddAttribute("r", tocstr(mat.diffuse.x()));
		node->AddAttribute("g", tocstr(mat.diffuse.y()));
		node->AddAttribute("b", tocstr(mat.diffuse.z()));
		node->AddAttribute("a", tocstr(mat.diffuse.w()));

		node_props->AddNode(node);
	}

	//Specular colour
	{
		auto node = file->AllocateNode("specular");

		node->AddAttribute("r", tocstr(mat.specular.x()));
		node->AddAttribute("g", tocstr(mat.specular.y()));
		node->AddAttribute("b", tocstr(mat.specular.z()));
		node->AddAttribute("a", tocstr(mat.specular.w()));

		node_props->AddNode(node);
	}

	//Shininess
	{
		auto node = file->AllocateNode("shininess", tocstr(mat.shininess));
		node_props->AddNode(node);
	}

	//Alpha
	{
		auto node = file->AllocateNode("alpha", tocstr(mat.alpha));
		node_props->AddNode(node);
	}

	matnode->AddNode(node_props);

	//Textures
	auto node_textures = file->AllocateNode("Textures");

	int j = 0;

	for (int i = 0; i < MATERIAL_TEXTURES_COUNT; i++)
	{
		if (mat.textures[i] != "")
		{
			auto node = file->AllocateNode(((string)"Texture" + tocstr(j)).c_str());

			node->AddAttribute("index", tocstr(i));
			node->AddAttribute("file", mat.textures[i].c_str());

			node_textures->AddNode(node);

			j++;
		}
	}

	matnode->AddNode(node_textures);
	if (add_node)
		pImpl->materialNode->AddNode(matnode);


	cout << "Material written - " << matnode->Name() << endl;

	pImpl->modified = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////

bool ParseVectorFromNode(Vector& v, FileSystem::IXMLnode* node)
{
	if (!node) return false;

	const char* val = nullptr;

	val = node->GetAttribute("x").value;
	if (!val) return false;
	v.x() = stof(val);

	val = node->GetAttribute("y").value;
	if (!val) return false;
	v.y() = stof(val);

	val = node->GetAttribute("z").value;
	if (val) v.z() = stof(val);

	val = node->GetAttribute("w").value;
	if (val) v.w() = stof(val);

	return true;
}

bool VECTOR_CALL WriteVectorToNode(Vector v, FileSystem::IXMLnode* node)
{
	if (!node) return false;

	node->AddAttribute("x", tocstr(v.x()));

	return true;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Material system helpers
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool ParseColourFromNode(FileSystem::IXMLnode* node, Vector& vec)
{
	if (!node) return false;

	const char* val = nullptr;

	val = node->GetAttribute("r").value;
	if (!val) return false;
	vec.x() = stof(val);

	val = node->GetAttribute("g").value;
	if (!val) return false;
	vec.y() = stof(val);

	val = node->GetAttribute("b").value;
	if (val) vec.z() = stof(val);

	val = node->GetAttribute("a").value;
	if (val) vec.w() = stof(val);

	return true;
}

static bool ParseVectorFromNode(FileSystem::IXMLnode* node, Vector& vec)
{
	if (!node) return false;

	const char* val = nullptr;

	val = node->GetAttribute("x").value;
	if (!val) return false;
	vec.x() = stof(val);

	val = node->GetAttribute("y").value;
	if (!val) return false;
	vec.y() = stof(val);

	val = node->GetAttribute("z").value;
	if (val) vec.z() = stof(val);

	val = node->GetAttribute("w").value;
	if (val) vec.w() = stof(val);

	return true;
}

static bool WriteVectorToNode(FileSystem::IXMLnode* node, Vector vec)
{
	if (node)
	{
		node->AddAttribute("x", tocstr(vec.x()));
		node->AddAttribute("y", tocstr(vec.y()));
		node->AddAttribute("z", tocstr(vec.z()));
		node->AddAttribute("w", tocstr(vec.w()));

		return true;
	}

	return false;
}

static bool WriteColourToNode(FileSystem::IXMLnode* node, Vector vec)
{
	if (node)
	{
		node->AddAttribute("r", tocstr(vec.x()));
		node->AddAttribute("g", tocstr(vec.y()));
		node->AddAttribute("b", tocstr(vec.z()));
		node->AddAttribute("a", tocstr(vec.w()));

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Material system
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ExportMaterials(const char* file, const vector<ModelMaterial>& materials)
{
	if (FileSystem::FileExists(file))
	{
		ofstream f(file, ios::trunc);
	}

	FileSystem fs;
	auto xml = fs.OpenXMLfile(file);

	if (xml)
	{
		auto rootnode = xml->AllocateNode("Materials");

		for (const ModelMaterial& mat : materials)
		{
			auto matnode = xml->AllocateNode(mat.name);
			FileSystem::IXMLnode* node = nullptr;

			//Colours
			node = xml->AllocateNode("Ambient");
			WriteColourToNode(node, mat.cAmbient);
			matnode->AddNode(node);
			node = xml->AllocateNode("Diffuse");
			WriteColourToNode(node, mat.cDiffuse);
			matnode->AddNode(node);
			node = xml->AllocateNode("Emissive");
			WriteColourToNode(node, mat.cEmissive);
			matnode->AddNode(node);
			node = xml->AllocateNode("Specular");
			WriteColourToNode(node, mat.cSpecular);
			matnode->AddNode(node);
			
			//Properties
			node = xml->AllocateNode("SpecularPower");
			node->AddAttribute("value", tocstr(mat.shininess));
			matnode->AddNode(node);
			node = xml->AllocateNode("Alpha");
			node->AddAttribute("value", tocstr(mat.alpha));
			matnode->AddNode(node);

			//Textures
			node = xml->AllocateNode("TextureDiffuse");
			node->AddAttribute("file", mat.texDiffuse);
			matnode->AddNode(node);
			node = xml->AllocateNode("TextureAmbient");
			node->AddAttribute("file", mat.texAmbient);
			matnode->AddNode(node);
			node = xml->AllocateNode("TextureDisplacement");
			node->AddAttribute("file", mat.texDisplacement);
			matnode->AddNode(node);
			node = xml->AllocateNode("TextureNormal");
			node->AddAttribute("file", mat.texNormal);
			matnode->AddNode(node);
			node = xml->AllocateNode("TextureSpecular");
			node->AddAttribute("file", mat.texSpecular);
			matnode->AddNode(node);

			rootnode->AddNode(matnode);
		}
		xml->Root()->AddNode(rootnode);
		xml->Save();

		return true;
	}

	return false;
}

bool ImportMaterials(const char* file, vector<ModelMaterial>& materials)
{
	if (!FileSystem::FileExists(file)) return false;

	FileSystem fs;
	auto xml = fs.OpenXMLfile(file);

	if (xml)
	{
		auto rootnode = xml->Root()->GetNode("Materials");

		for (auto i = rootnode->begin(); i != rootnode->end(); i++)
		{
			ModelMaterial mat;

			FileSystem::IXMLnode* node = nullptr;

			//Material name
			strcpy_s(mat.name, i->Name());

			//Colours
			ParseColourFromNode(i->GetNode("Ambient"), mat.cAmbient);
			ParseColourFromNode(i->GetNode("Diffuse"), mat.cDiffuse);
			ParseColourFromNode(i->GetNode("Emissive"), mat.cEmissive);
			ParseColourFromNode(i->GetNode("Specular"), mat.cSpecular);

			//Properties
			if (auto node = i->GetNode("SpecularPower"))
				mat.shininess = stof(node->GetAttribute("value").value);
			if (auto node = i->GetNode("Alpha"))
				mat.alpha = stof(node->GetAttribute("value").value);

			//Textures
			if (auto node = i->GetNode("TextureDiffuse"))
				strcpy_s(mat.texDiffuse, node->GetAttribute("file").value);
			if (auto node = i->GetNode("TextureAmbient"))
				strcpy_s(mat.texAmbient, node->GetAttribute("file").value);
			if (auto node = i->GetNode("TextureDisplacement"))
				strcpy_s(mat.texDisplacement, node->GetAttribute("file").value);
			if (auto node = i->GetNode("TextureNormal"))
				strcpy_s(mat.texNormal, node->GetAttribute("file").value);
			if (auto node = i->GetNode("TextureSpecular"))
				strcpy_s(mat.texSpecular, node->GetAttribute("file").value);

			materials.push_back(mat);
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ComputeBoundingBox(const Vertex* vptr, uint32 size, BoundingBox& bb)
{
	bb = BoundingBox();

	for (uint32 i = 0; i < size; i++)
	{
		const Vector& vec = vptr[i].get((uint32)VertexAttributeIndex::Position);

		if (vec.x() > bb.max.x()) bb.max.x() = vec.x();
		if (vec.y() > bb.max.y()) bb.max.y() = vec.y();
		if (vec.z() > bb.max.z()) bb.max.z() = vec.z();

		if (vec.x() < bb.min.x()) bb.min.x() = vec.x();
		if (vec.y() < bb.min.y()) bb.min.y() = vec.y();
		if (vec.z() < bb.min.z()) bb.min.z() = vec.z();
	}
}

void ComputeBoundingBoxIndexed(const Vertex* vptr, const Index* iptr, uint32 size, BoundingBox& bb)
{
	bb = BoundingBox();

	for (uint32 i = 0; i < size; i++)
	{
		const Vector& vec = vptr[iptr[i]].get((uint32)VertexAttributeIndex::Position);

		if (vec.x() > bb.max.x()) bb.max.x() = vec.x();
		if (vec.y() > bb.max.y()) bb.max.y() = vec.y();
		if (vec.z() > bb.max.z()) bb.max.z() = vec.z();

		if (vec.x() < bb.min.x()) bb.min.x() = vec.x();
		if (vec.y() < bb.min.y()) bb.min.y() = vec.y();
		if (vec.z() < bb.min.z()) bb.min.z() = vec.z();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////