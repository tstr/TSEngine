/*
	Model format definitions and types

	format is as follows:

	[ModelHeader]
	[Mesh0]
	[Mesh1]
	...
	[MeshN]
	[Vertices]
	[Indices]
*/

#pragma once

#include <tscore/types.h>
#include <tscore/maths.h>
#include <tscore/strings.h>

namespace ts
{
	typedef uint32 ModelIndex;
	
	enum { MaxMaterialNameLength = 64 };
	
	struct SModelHeader
	{
		uint32 numMeshes = 0;
		ModelIndex numVertices = 0;
		ModelIndex numIndices = 0;
		uint32 reserved = 0;
	};
	
	struct SModelMesh
	{
		ModelIndex indexOffset = 0;
		ModelIndex indexCount = 0;
		uint8 vertexAttributeMask = 0;
		StaticString<MaxMaterialNameLength> materialName;
	};
	
	enum EModelVertexAttributes : uint8
	{
		eModelVertexAttributePosition  = 0x01,
		eModelVertexAttributeTexcoord  = 0x02,
		eModelVertexAttributeColour	   = 0x04,
		eModelVertexAttributeNormal	   = 0x08,
		eModelVertexAttributeTangent   = 0x10,
		eModelVertexAttributeBitangent = 0x20,
	};

	struct SModelVertex
	{
		Vector position;
		Vector texcoord;
		Vector colour;
		Vector normal;
		Vector tangent;
		Vector bitangent;
	};
}
