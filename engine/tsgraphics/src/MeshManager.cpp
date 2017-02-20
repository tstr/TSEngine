/*
	Mesh manager source
*/

#include <tsgraphics/MeshManager.h>

#include <tsgraphics/api/RenderApi.h>
#include <tscore/table.h>

#include <map>
#include <vector>

using namespace std;
using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CMeshManager::Impl
{
	GraphicsCore* system = nullptr;
	Table<SMeshInstance, MeshId> meshPool;

	void clearPool()
	{
		if (!system)
		{
			return;
		}

		IRender* api = system->getApi();

		for (SMeshInstance& inst : meshPool.getArray())
		{
			if (inst.indexBuffer != HBUFFER_NULL)
			{
				api->destroyBuffer(inst.indexBuffer);
				inst.indexBuffer = HBUFFER_NULL;
			}

			for (uint32 i = 0; i < SDrawCommand::eMaxVertexBuffers; i++)
			{
				if (inst.vertexBuffers[i] != HBUFFER_NULL)
				{
					api->destroyBuffer(inst.vertexBuffers[i]);
					inst.vertexBuffers[i] = HBUFFER_NULL;
				}
			}
		}

		meshPool.clear();
	}

	Impl(GraphicsCore* core) :
		system(core)
	{

	}

	~Impl()
	{
		clearPool();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMeshManager::CMeshManager(GraphicsCore* system) :
	pManage(new Impl(system))
{

}

CMeshManager::~CMeshManager()
{
	pManage.reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EMeshStatus CMeshManager::createMesh(SVertexMesh& mesh, MeshId& id)
{
	if (!pManage)
	{
		return eMeshStatus_Fail;
	}

	auto api = pManage->system->getApi();

	SMeshInstance inst;
	
	//Vertex buffer
	SBufferResourceData vdesc;
	vdesc.memory = &mesh.vertexData[0];
	vdesc.size = (uint32)mesh.vertexData.size();
	vdesc.usage = eBufferTypeVertex;
	if (auto r = api->createResourceBuffer(inst.vertexBuffers[0], vdesc))
		return eMeshStatus_Fail;

	inst.vertexOffset[0] = 0;
	inst.vertexStrides[0] = mesh.vertexStride;
	inst.vertexCount = (uint32)mesh.vertexData.size();
	inst.topology = mesh.vertexTopology;

	//Index buffer
	SBufferResourceData idesc;
	idesc.memory = &mesh.indexData[0];
	idesc.size = (uint32)mesh.indexData.size() * sizeof(Index);
	idesc.usage = eBufferTypeIndex;
	if (auto r = api->createResourceBuffer(inst.indexBuffer, idesc))
		return eMeshStatus_Fail;

	inst.indexCount = (uint32)mesh.indexData.size();

	uint32 attribIdx = 0;
	for (SVertexAttribute& attrib : mesh.vertexAttributes)
	{
		inst.vertexAttributes[attribIdx] = attrib;
		attribIdx++;
	}

	inst.vertexAttributeCount = (uint32)mesh.vertexAttributes.size();

	pManage->meshPool.create(inst, id);

	return eMeshStatus_Ok;
}

EMeshStatus CMeshManager::getMeshInstance(MeshId id, SMeshInstance& inst)
{
	size_t idx = (size_t)id - 1;


	if (!pManage || !pManage->meshPool.get(id, inst))
	{
		return eMeshStatus_Fail;
	}

	return eMeshStatus_Ok;
}

void CMeshManager::clear()
{
	if (pManage)
	{
		pManage->clearPool();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
