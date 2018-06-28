/*
	Model object
*/

#include <tsgraphics/Model.h>
#include <tsgraphics/schemas/Model.rcs.h>

#include <fstream>

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

bool Model::load(RenderDevice* device, const String& filePath)
{
	rc::ResourceLoader loader(std::ifstream(filePath, std::ios::binary));
	auto& modelReader = loader.deserialize<tsr::Model>();

	if (loader.fail())
	{
		return false;
	}

	//There must be vertex data
	if (!modelReader.has_vertexData())
	{
		return false;
	}

	m_vertices = Buffer::create(
		device,
		modelReader.vertexData().data(),
		modelReader.vertexData().size(),
		BufferType::VERTEX
	);

	if (modelReader.has_indexData())
	{
		m_indices = Buffer::create(
			device,
			modelReader.indexData().data(),
			modelReader.indexData().size() * sizeof(uint32),
			BufferType::INDEX
		);
	}

	//Copy attributes
	if (modelReader.has_attributeNames())
	{
		for (uint32 i = 0; i < modelReader.attributeNames().size(); i++)
		{
			m_attributes[modelReader.attributeNames()[i].str()] = modelReader.attributeOffsets()[i];
		}
	}

	//Iterate over model meshes
	for (uint32 i = 0; i < modelReader.meshes().length(); i++)
	{
		const auto& meshReader = modelReader.meshes()[i];

		Mesh mesh;

		mesh.name = meshReader.materialName().str();
		mesh.indices = m_indices.handle();
		mesh.vertices = m_vertices.handle();

		mesh.vertexStride = modelReader.vertexStride();
		mesh.vertexBase = meshReader.vertexBase();
		mesh.vertexStart = 0;
		mesh.vertexCount = meshReader.vertexCount();

		mesh.indexStart = meshReader.indexOffset();
		mesh.indexCount = meshReader.indexCount();

		mesh.mode = (mesh.indexCount > 0) ? DrawMode::INDEXED : DrawMode::VERTEX;

		m_meshes.push_back(mesh);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
