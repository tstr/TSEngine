/*
	Vertex builder class implementation
*/

#include <tsgraphics/MeshManager.h>
#include <tscore/maths.h>

#include <sstream>

using namespace std;
using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Return size in bytes of a vertex attribute of a given type
static uint32 getAttributeTypeSize(EVertexAttributeType type);

//Marks whether begin()/end() has been called
//The state setter methods can only be called inbetween begin()/end() calls
enum EVertexBuilderStatus
{
	eVertexBuilderStatus_Inactive = 0,
	eVertexBuilderStatus_Active	  = -1
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Begin/End methods
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVertexBuilder::begin(uint32 vertexCount, uint32 indexCount)
{
	if (m_status == eVertexBuilderStatus_Active)
	{
		return;
	}

	m_attributeMap.clear();
	m_vertexStreams.clear();
	m_indexStream.clear();

	m_stateVertexCount = vertexCount;
	m_stateIndexCount = indexCount;

	m_status = eVertexBuilderStatus_Active;
}

void CVertexBuilder::end(SVertexMesh& output)
{
	if (m_status == eVertexBuilderStatus_Inactive)
	{
		return;
	}

	//Copy index data
	output.indexData = this->m_indexStream;
	
	//Interleave vertex attribute streams into a single buffer
	
	uint32 bufferSlot = 0;
	uint32 byteOffset = 0;
	
	//Create vertex attribute descriptors - used by shader program
	for (SVertexStream& attribStream : m_vertexStreams)
	{
		SVertexAttribute descriptor;
		descriptor.bufferSlot = bufferSlot;
		descriptor.byteOffset = byteOffset;
		descriptor.channel = EVertexAttributeChannel::eChannelPerVertex;
		descriptor.semanticName = attribStream.attributeName.str();
		descriptor.type = attribStream.type;

		output.vertexAttributes.push_back(descriptor);

		//Increment byte offset
		byteOffset += attribStream.dataStride;
	}

	//The bytewidth of the interleaved vertex buffer is equal to the byteoffset pointer
	output.vertexStride = byteOffset;

	//Preallocate vertex data buffer
	output.vertexData.resize(output.vertexStride * m_stateVertexCount);

	stringstream vertexbuffer(ios::binary | ios::in | ios::out);

	for (uint32 i = 0; i < m_stateVertexCount; i++)
	{
		//For each attribute stream write each attribute into the vertex buffer
		for (SVertexStream& s : m_vertexStreams)
		{
			byte* src = &s.streamData[i * s.dataStride];

			vertexbuffer.write((const char*)src, s.dataStride);
		}
	}

	//Copy vertex buffer into our output struct
	vertexbuffer.read((char*)&output.vertexData[0], output.vertexData.size());

	m_status = eVertexBuilderStatus_Inactive;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	State setters
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVertexBuilder::setIndexStream(const Index* data)
{
	if (m_stateIndexCount == 0 || m_status == eVertexBuilderStatus_Inactive)
	{
		return false;
	}

	m_indexStream = vector<Index>(data, data + m_stateIndexCount);

	return true;
}

bool CVertexBuilder::setAttributeStream(const VertexAttributeString& name, const byte* data, uint32 stride, EVertexAttributeType type)
{
	if (m_stateVertexCount == 0 || m_status == eVertexBuilderStatus_Inactive)
	{
		return false;
	}

	auto it = m_attributeMap.find(name);
	uint32 streamIndex = 0;
	
	//If a vertex stream with this attribute name does not exist
	if (it == m_attributeMap.end())
	{	
		//Create a new vertex stream and store its index
		m_vertexStreams.push_back(SVertexStream());
		streamIndex = (uint32)m_vertexStreams.size() - 1;
		m_attributeMap[name] = streamIndex;
	}
	else
	{
		//Set stream index to the existing index and overwrite old stream
		streamIndex = it->second;
		m_vertexStreams[streamIndex].attributeName = "";
		m_vertexStreams[streamIndex].dataStride = 0;
		m_vertexStreams[streamIndex].streamData.clear();
		m_vertexStreams[streamIndex].type = eAttribUnknown;
	}

	SVertexStream& attribStream = m_vertexStreams[streamIndex];

	const uint32 attribStride = getAttributeTypeSize(type);

	attribStream.attributeName = name;
	attribStream.dataStride = attribStride;
	attribStream.streamData.resize(m_stateVertexCount * attribStride);
	attribStream.type = type;

	//Iterate over input stream and copy into output stream
	for (uint32 i = 0; i < m_stateVertexCount; i++)
	{
		//Read pointer
		const byte* src = data + (i * stride);
		//Write pointer
		byte* dest = &attribStream.streamData[i * attribStride];

		memcpy(dest, src, attribStride);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 getAttributeTypeSize(EVertexAttributeType type)
{
	switch (type)
	{
		case eAttribFloat: return sizeof(float);
		case eAttribFloat2: return sizeof(float) * 2;
		case eAttribFloat3: return sizeof(float) * 3;
		case eAttribFloat4: return sizeof(float) * 4;

		case eAttribMatrix: return sizeof(float) * 16;

		case eAttribInt32: return sizeof(int32);
		case eAttribUint32: return sizeof(uint32);

		case eAttribRGBA: return 4;
		case eAttribRGB: return 3;

		default: return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
