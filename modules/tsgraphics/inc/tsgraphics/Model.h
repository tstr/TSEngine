/*
	Model object
*/

#pragma once

#include <tscore/types.h>
#include <vector>
#include <unordered_map>

#include "Driver.h"
#include "Buffer.h"

namespace ts
{
	using VertexAttributeMap = std::unordered_map<String, uint32>;

	struct Mesh
	{
		String name;

		ResourceHandle vertices = ResourceHandle();
		ResourceHandle indices = ResourceHandle();

		uint32 vertexStride = 0;
		int32 vertexBase = 0;

		uint32 vertexCount = 0;
		uint32 vertexStart = 0;

		uint32 indexCount = 0;
		uint32 indexStart = 0;

		DrawMode mode;



	};

	class Model
	{
	public:

		Model() {}
		Model(const Model&) = delete;

		Model(Model&& rhs) :
			m_attributes(std::move(rhs.m_attributes)),
			m_meshes(std::move(rhs.m_meshes))
		{}

		Model(RenderDevice* device, const String& filePath) { load(device, filePath); }

		bool load(RenderDevice* device, const String& filePath);

		const VertexAttributeMap& attributes() const { return m_attributes; }

		const std::vector<Mesh>& meshes() const { return m_meshes; }

	private:

		Buffer m_vertices;
		Buffer m_indices;

		VertexAttributeMap m_attributes;
		std::vector<Mesh> m_meshes;
	};
}
