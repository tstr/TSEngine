/*
	Graphics effect source
*/

#include "pch.h"

#include <C3Ext\win32.h>
#include <C3E\gfx\graphics.h>
#include <C3E\gfx\abi\graphicsabi.h>
#include <C3E\core\strings.h>

#include <C3E\filesystem\filecore.h>
#include <C3E\filesystem\filexml.h>

#include <iomanip>

#include "libraryloader.h"

using namespace std;
using namespace C3E;
using namespace C3E::ABI;

static const char fx_extension[] = "fx";
static const char fx_cache_extension[] = "cfx";
static const char fx_def_extension[] = "fxm";

enum
{
	MAX_SEMANTIC_NAME_LENGTH = 32,
	MAX_ENTRYPOINT_NAME_LENGTH = 32
};

enum SemanticType
{
	SEMANTIC_UNKNOWN,
	SEMANTIC_FLOAT,
	SEMANTIC_FLOAT2,
	SEMANTIC_FLOAT3,
	SEMANTIC_FLOAT4,
	SEMANTIC_INT,
	SEMANTIC_UINT,
	SEMANTIC_MATRIX,
};

struct EffectFileHeader
{
	int shader_type_mask = ABI::SHADER_TYPE_UNKNOWN;
	int num_semantics = 0;
	int num_shaders;
};

struct EffectFileSemantic
{
	char semanticName[MAX_SEMANTIC_NAME_LENGTH];
	int semanticType;
};

struct EffectFileShader
{
	int shader_type = 0;
	size_t shader_size = 0;
};

//float4 vector offset - 16bytes
#define VECTOR_OFFSET(x) (sizeof(Vector) * x)

string int_to_hex(uint64 i)
{
	stringstream stream;
	stream << setfill('0') << setw(sizeof(int) * 2) << hex << i;
	return stream.str();
}

#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
uint64 hash_str(const char* s)
{
	uint64 h = 31 /* also prime */;
	while (*s) {
		h = (h * A) ^ (s[0] * B);
		s++;
	}
	return h; // or return h % C;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EffectFactory::Impl
{
	Graphics* m_renderer = nullptr;
	GraphicsBackend m_backend;

	mutex m_mutex;
	map<string, shared_ptr<Effect>> m_effects;

	char m_workingDirectory[MAX_PATH]; //Working directory
	char m_outputDirectory[MAX_PATH]; //Output directory

	int m_flags = 0;
};

struct Effect::Impl
{
	Graphics* m_renderer = nullptr;

	ResourceHandle m_vertexShaderProxy = 0;
	ResourceHandle m_pixelShaderProxy = 0;
	ResourceHandle m_geometryShaderProxy = 0;
	ResourceHandle m_sid = 0;

	Impl(Graphics* renderer) : m_renderer(renderer)
	{
		C3E_ASSERT(m_renderer);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Effect methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Effect::Effect(
	Graphics* renderer,
	istream& cache
) : pImpl(new Impl(renderer))
{
	auto api = (IRenderApi*)pImpl->m_renderer->api();

	if (cache.fail())
	{
		throw runtime_error("Effect stream could not be read");
	}

	EffectFileHeader header;
	ZeroMemory(&header, sizeof(EffectFileHeader));

	cache.read(reinterpret_cast<char*>(&header), sizeof(EffectFileHeader));

	if (cache.fail())
	{
		throw runtime_error("Effect stream could not be read");
	}

	cout << "Loading effect cache...\n";
	cout << "Shader semantic count: " << header.num_semantics << endl;
	cout << "Shader count: " << header.num_shaders << endl;
	cout << "Shader type mask: " << header.shader_type_mask << endl;

	vector<EffectFileSemantic> semantics(header.num_semantics);

	if (header.num_semantics > 0)
	{
		for (int i = 0; i < header.num_semantics; i++)
		{
			EffectFileSemantic s;
			cache.read(reinterpret_cast<char*>(&s), sizeof(EffectFileSemantic));
			semantics[i] = s;
		}
	}

	vector<byte> vertex_bytecode;

	for (int i = 0; i < header.num_shaders; i++)
	{
		EffectFileShader sh;
		cache.read(reinterpret_cast<char*>(&sh), sizeof(EffectFileShader));

		auto x = cache.tellg();

		if (cache.fail())
		{
			throw runtime_error("Effect stream could not be read");
		}

		vector<byte> bytecode;
		bytecode.resize(sh.shader_size);
		cache.read((char*)&bytecode[0], bytecode.size());

		if (cache.fail())
		{
			throw runtime_error("Effect stream could not be read");
		}

		switch (sh.shader_type)
		{
		case (SHADER_TYPE_VERTEX) :
		{
			vertex_bytecode = bytecode;
			pImpl->m_vertexShaderProxy = api->CreateShader(&bytecode[0], bytecode.size(), SHADER_TYPE_VERTEX);
			break;
		}
		case(SHADER_TYPE_PIXEL) :
		{
			pImpl->m_pixelShaderProxy = api->CreateShader(&bytecode[0], bytecode.size(), SHADER_TYPE_PIXEL);
			break;
		}
		case(SHADER_TYPE_GEOMETRY) :
		{
			pImpl->m_geometryShaderProxy = api->CreateShader(&bytecode[0], bytecode.size(), SHADER_TYPE_GEOMETRY);
			break;
		}
		default:
		{
			throw exception("");
		}
		}
	}

	cout << "Cache loaded\n";

	ShaderInputDescriptor sid[(uint32)VertexAttributeIndex::MaxAttributeCount];

	int i = 0;

	if (!semantics.empty())
	{
		for (EffectFileSemantic& semantic : semantics)
		{
			sid[i].semanticName = semantic.semanticName;
			sid[i].slot = 0;

			string sname(semantic.semanticName);

			if (sname == "POSITION")
			{
				sid[i].byteOffset = VECTOR_OFFSET((uint32)VertexAttributeIndex::Position);
				sid[i].vectorComponents = 3;
			}
			else if (sname == "NORMAL")
			{
				sid[i].byteOffset = VECTOR_OFFSET((uint32)VertexAttributeIndex::Normal);
				sid[i].vectorComponents = 3;
			}
			else if (sname == "TEXCOORD")
			{
				sid[i].byteOffset = VECTOR_OFFSET((uint32)VertexAttributeIndex::Texcoord);
				sid[i].vectorComponents = 2;
			}
			else if (sname == "COLOUR")
			{
				sid[i].byteOffset = VECTOR_OFFSET((uint32)VertexAttributeIndex::Colour);
				sid[i].vectorComponents = 4;
			}
			else if (sname == "TANGENT")
			{
				sid[i].byteOffset = VECTOR_OFFSET((uint32)VertexAttributeIndex::Tangent);
				sid[i].vectorComponents = 4;
			}
			else if (sname == "BITANGENT")
			{
				sid[i].byteOffset = VECTOR_OFFSET((uint32)VertexAttributeIndex::Bitangent);
				sid[i].vectorComponents = 3;
			}

			i++;
		}

		pImpl->m_sid = api->CreateShaderInputDescriptor(&vertex_bytecode[0], vertex_bytecode.size(), sid, (uint32)semantics.size());
	}
}

Effect::~Effect()
{
	auto api = (IRenderApi*)pImpl->m_renderer->api();

	if (pImpl->m_vertexShaderProxy)
		api->DestroyShader(pImpl->m_vertexShaderProxy);

	if (pImpl->m_pixelShaderProxy)
		api->DestroyShader(pImpl->m_pixelShaderProxy);

	if (pImpl->m_geometryShaderProxy)
		api->DestroyShader(pImpl->m_geometryShaderProxy);

	if (pImpl->m_sid)
		api->DestroyShaderInputDescriptor(pImpl->m_sid);

	if (pImpl) delete pImpl;
}

ResourceHandle Effect::GetShader(uint32 stage) const
{
	switch (stage)
	{
		case ((uint32)ShaderType::SHADER_TYPE_VERTEX) :
		{
			return pImpl->m_vertexShaderProxy;
		}
		case ((uint32)ShaderType::SHADER_TYPE_PIXEL) :
		{
			return pImpl->m_pixelShaderProxy;
		}
		case ((uint32)ShaderType::SHADER_TYPE_GEOMETRY) :
		{
			return pImpl->m_geometryShaderProxy;
		}
	}

	return nullptr;
}

ResourceHandle Effect::GetShaderInputDescriptor() const
{
	return pImpl->m_sid;
}

EffectType Effect::GetType() const
{
	return EffectType::TypeMaterial;
}

uint32 Effect::GetVertexAttributeMask() const
{
	return ((uint32)VertexAttributes::Position | (uint32)VertexAttributes::Normal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Effect Factory methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EffectFactory::EffectFactory(Graphics* renderer, int flags) :
	pImpl(new Impl)
{
	pImpl->m_renderer = renderer;
	pImpl->m_backend.load(renderer->apiEnum());
}

EffectFactory::~EffectFactory()
{
	if (pImpl)
		delete pImpl;
}

void EffectFactory::CreateEffect(const char* file, shared_ptr<Effect>& fx)
{
	//lock_guard<mutex>lk(pImpl->m_mutex);
	string id(int_to_hex(hash_str(file)));

	auto i = pImpl->m_effects.find(id);
	
	if (i == pImpl->m_effects.end())
	{
		string path((string)pImpl->m_outputDirectory + id + "." + fx_cache_extension);

		if (FileSystem::FileExists(path.c_str()))
		{
			cout << "Effect cache found: """ << file << """\n";
			fx.reset(new Effect(pImpl->m_renderer, ifstream(path, ios::binary)));
			pImpl->m_effects[id] = fx;
		}
		else
		{
			cout << "Effect cache was not found: """ << file << """\n";
		}
	}
	else
	{
		fx = i->second;
	}
}

int EffectFactory::CompileEffectManifest(const char* manifest)
{
	FileSystem fs;

	string f(manifest);
	string directory;
	if (!FileSystem::FileExists(f.c_str()))
	{
		directory = pImpl->m_workingDirectory;
		f = directory + f;
	}

	cout << "Compiling effect manifest: """ + f + """\n";

	unique_ptr<FileSystem::IXMLfile> xml(fs.OpenXMLfile(f.c_str()));

	typedef FileSystem::IXMLnode node_t;
	typedef node_t::iterator node_iterator;

	node_t* node_fx = xml->Root()->GetNode("Effects");

	/////////////////////////////////////////////////////////////////////////////////

	struct ShaderTarget
	{
		char entrypoint[32];
		ShaderType type;
	};

	if (node_fx)
	{
		for (node_iterator node = node_fx->begin(); node != node_fx->end(); node++)
		{
			//Source file path for the effect
			node_t::Attribute a = node->GetAttribute("file");

			if (!a.name || !a.value)
				continue;

			vector<ShaderTarget> targets;
			int shaderTypeMask = 0;

			/////////////////////////////////////////////////////////////////////////////////

			node_t* node_shader_root = nullptr;
			node_t* node_shader = nullptr;

			node_shader_root = node->GetNode("Stages");

			if (node_shader_root)
			{
				node_t* node = node_shader_root;

				node_shader = node->GetNode("Pixel");
				if (node_shader)
				{
					ShaderTarget t;
					t.type = ABI::ShaderType::SHADER_TYPE_PIXEL;
					shaderTypeMask |= t.type;

					node_t::Attribute a = node_shader->GetAttribute("entrypoint");
					strncpy_s(t.entrypoint, a.value, strlen(a.value));

					targets.push_back(t);
				}

				node_shader = node->GetNode("Vertex");
				if (node_shader)
				{
					ShaderTarget t;
					t.type = ABI::ShaderType::SHADER_TYPE_VERTEX;
					shaderTypeMask |= t.type;

					node_t::Attribute a = node_shader->GetAttribute("entrypoint");
					strncpy_s(t.entrypoint, a.value, strlen(a.value));

					targets.push_back(t);
				}

				node_shader = node->GetNode("Geometry");
				if (node_shader)
				{
					ShaderTarget t;
					t.type = ABI::ShaderType::SHADER_TYPE_GEOMETRY;
					shaderTypeMask |= t.type;

					node_t::Attribute a = node_shader->GetAttribute("entrypoint");
					strncpy_s(t.entrypoint, a.value, strlen(a.value));

					targets.push_back(t);
				}

				node_shader = node->GetNode("Compute");
				if (node_shader)
				{
					ShaderTarget t;
					t.type = ABI::ShaderType::SHADER_TYPE_COMPUTE;
					shaderTypeMask |= t.type;

					node_t::Attribute a = node_shader->GetAttribute("entrypoint");
					strncpy_s(t.entrypoint, a.value, strlen(a.value));

					targets.push_back(t);
				}
			}
			else
			{
				cout << "Invalid node\n";
				continue;
			}

			node_t* node_semantics = nullptr;
			uint32 attribute_mask = 0;

			node_semantics = node->GetNode("Semantics");

			vector<EffectFileSemantic> semantics;

			if (node_semantics)
			{
				for (auto node_semantic = node_semantics->begin(); node_semantic != node_semantics->end(); node_semantic++)
				{
					const char* val = node_semantic->GetAttribute("semantic").value;

					EffectFileSemantic s;
					ZeroMemory(&s, sizeof(EffectFileSemantic));
					strcpy_s(s.semanticName, val);
					s.semanticType = SEMANTIC_FLOAT4;
					semantics.push_back(s);
				}
			}

			/////////////////////////////////////////////////////////////////////////////////
			
			string source_name(directory + a.value);
			ifstream sourcestream(source_name);

			if (sourcestream.fail())
			{
				continue;
			}

			istreambuf_iterator<char> eos;
			string sourcestring(istreambuf_iterator<char>(sourcestream), eos);
			sourcestream.close();

			ofstream out((string)pImpl->m_outputDirectory + int_to_hex(hash_str(node->Name())) + "." + fx_cache_extension, ios::binary);

			EffectFileHeader fileheader;
			ZeroMemory(&fileheader, sizeof(EffectFileHeader));

			fileheader.shader_type_mask = shaderTypeMask;
			fileheader.num_semantics = (int)semantics.size();
			fileheader.num_shaders = (int)targets.size();

			if (out.fail())
				continue;

			int flags = 0;

			if (pImpl->m_flags & EffectFactory::FX_DEBUG)
			{
				flags |= (int)ABI::ShaderCompileFlag::SHADER_COMPILE_DEBUG;
			}

			//Write the file header
			out.write(reinterpret_cast<const char*>(&fileheader), sizeof(fileheader));

			if (!semantics.empty())
			for (EffectFileSemantic& s : semantics)
			{
				out.write(reinterpret_cast<const char*>(&s), sizeof(EffectFileSemantic));
			}

			for (ShaderTarget& target : targets)
			{
				size_t size = 0;
				stringstream outbuf(ios::out | ios::in);
				stringstream errbuf(ios::out | ios::in);

				if (!pImpl->m_backend.f_compile(source_name.c_str(), sourcestring.c_str(), sourcestring.size(), outbuf, errbuf, size, target.entrypoint, target.type, flags))
				{
					cerr << "Effect error: """ << source_name << """\n";
					cerr << errbuf.rdbuf();

					return false;
				}

				EffectFileShader shader_header;
				shader_header.shader_type = target.type;
				shader_header.shader_size = size;

				out.write(reinterpret_cast<const char*>(&shader_header), sizeof(EffectFileShader));
				
				if (out.fail())
				{
					throw exception("Effect compiler");
				}

				out << outbuf.rdbuf();
			}

			if (out.fail())
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////

	cout << "Effect compiled\n";

	return true;
}

void EffectFactory::SetWorkingDirectory(const char* path)
{
	strcpy_s(pImpl->m_workingDirectory, path);
}

void EffectFactory::SetOutputDirectory(const char* path)
{
	strcpy_s(pImpl->m_outputDirectory, path);
}

int EffectFactory::GetFlags() const
{
	return pImpl->m_flags;
}

void EffectFactory::SetFlags(int f)
{
	pImpl->m_flags = f;
}

Graphics* EffectFactory::GetRenderer() const
{
	return pImpl->m_renderer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////