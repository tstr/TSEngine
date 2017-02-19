/*
	Mesh Manager header
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/api/renderapi.h>
#include <tsgraphics/api/rendercommon.h>

#include <tscore/ptr.h>
#include <tscore/debug/assert.h>
#include <tscore/strings.h>

#include <vector>
#include <map>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class GraphicsSystem;

	typedef uint32 Index;

	/////////////////////////////////////////////////////////////////////////////////////////////////

	/*
		Vertex Mesh

		Structure that contains all of the data needed to represent a complete mesh
	*/
	struct SVertexMesh
	{
		std::vector<byte> vertexData;
		std::vector<Index> indexData;
		std::vector<SVertexAttribute> vertexAttributes;
		uint32 vertexStride = 0;
		EVertexTopology vertexTopology = EVertexTopology::eTopologyUnknown;
	};

	/*
		Vertex Builder class

		Constructs vertex/index buffers from one or more arrays of vertex attributes
	*/
	class CVertexBuilder
	{
	private:
		
		struct SVertexStream
		{
			VertexAttributeString attributeName;
			std::vector<byte> streamData;
			uint32 dataStride = 0;
			EVertexAttributeType type;
		};

		std::map<VertexAttributeString, uint32> m_attributeMap;
		std::vector<SVertexStream> m_vertexStreams;
		std::vector<Index> m_indexStream;

		uint32 m_stateVertexCount = 0;
		uint32 m_stateIndexCount = 0;

		int m_status = 0;

	public:

		TSGRAPHICS_API void begin(uint32 vertexCount, uint32 indexCount);
		TSGRAPHICS_API void end(SVertexMesh& output);

		TSGRAPHICS_API bool setIndexStream(const Index* data);
		TSGRAPHICS_API bool setAttributeStream(const VertexAttributeString& name, const byte* data, uint32 stride, EVertexAttributeType type);

		bool setIndexStream(const std::vector<Index>& data)
		{
			tsassert(data.size() >= m_stateIndexCount);
			this->setIndexStream(&data[0]);
			return true;
		}

		template<typename attrib_t>
		bool setAttributeStream(const VertexAttributeString& name, const std::vector<attrib_t>& data, EVertexAttributeType type)
		{
			tsassert(data.size() >= m_stateVertexCount);
			this->setAttributeStream(name, (const byte*)&data[0], (uint32)sizeof(attrib_t), type);
			return true;
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////

	enum EMeshStatus
	{
		eMeshStatus_Ok		= 0,
		eMeshStatus_Fail	= 1,
		eMeshStatus_NoMesh	= 2
	};

	struct SMeshInstance
	{
		HBuffer vertexBuffers[SDrawCommand::eMaxVertexBuffers] = { HBUFFER_NULL };
		HBuffer indexBuffer = HBUFFER_NULL;

		uint32 vertexStrides[SDrawCommand::eMaxVertexBuffers] = { 0 };
		uint32 vertexOffset[SDrawCommand::eMaxVertexBuffers] = { 0 };

		uint32 indexCount = 0;
		uint32 vertexCount = 0;

		SVertexAttribute vertexAttributes[SDrawCommand::eMaxVertexAttributes];
		uint32 vertexAttributeCount = 0;

		EVertexTopology topology = EVertexTopology::eTopologyUnknown;
	};

	typedef uint32 MeshId;

	class CMeshManager
	{
	private:

		struct Impl;
		OpaquePtr<Impl> pManage;

	public:

		OPAQUE_PTR(CMeshManager, pManage)

		CMeshManager() {}

		TSGRAPHICS_API CMeshManager(GraphicsSystem* system);
		TSGRAPHICS_API ~CMeshManager();

		TSGRAPHICS_API EMeshStatus createMesh(SVertexMesh& mesh, MeshId& id);
		TSGRAPHICS_API EMeshStatus getMeshInstance(MeshId id, SMeshInstance& inst);

		TSGRAPHICS_API void clear();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
