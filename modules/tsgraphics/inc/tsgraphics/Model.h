/*
	Model object
*/

#pragma once

#include <tscore/types.h>
#include <vector>
#include <unordered_map>

#include "Driver.h"
#include "Buffer.h"
#include "AssetCache.h"

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

		VertexAttributeMap vertexAttributes;
		VertexTopology vertexTopology;

		//Helper methods
		VertexBufferView getBufferView() const;
		DrawParams getParams() const;
	};

	class Model
	{
	public:

		Model() {}
		Model(const Model&) = delete;
		void operator=(const Model&) = delete;

		Model(Model&& rhs) :
			m_attributes(std::move(rhs.m_attributes)),
			m_meshes(std::move(rhs.m_meshes)), 
			m_materialFilePath(rhs.m_materialFilePath),
			m_modelFilePath(rhs.m_modelFilePath),
			m_vertices(std::move(rhs.m_vertices)),
			m_indices(std::move(rhs.m_indices)),
			m_error(rhs.m_error)
		{}

		Model(RenderDevice* device, const String& filePath) { load(device, filePath); }

		bool load(RenderDevice* device, const String& filePath);

		const VertexAttributeMap& attributes() const { return m_attributes; }

		const std::vector<Mesh>& meshes() const { return m_meshes; }

		// File path properties
		const Path& modelFile() const { return m_modelFilePath; }
		const Path& materialFile() const { return m_materialFilePath; }
		bool hasMaterialFile() const { return m_materialFilePath != ""; }

		// True if there was an error creating the model
		bool error() const { return m_error; }

	private:

		bool setError(bool err) { m_error = err; return m_error; }

		bool m_error = false;

		Buffer m_vertices;
		Buffer m_indices;

		VertexAttributeMap m_attributes;
		std::vector<Mesh> m_meshes;

		Path m_materialFilePath;
		Path m_modelFilePath;
	};


	/*
		Model loader cache
	*/
	class ModelCache : public AssetCache<ModelCache, Model>
	{
	public:

		ModelCache(RenderDevice* device = nullptr) : m_device(device) {}

		Model load(const Path& filePath)
		{
			Model model;
			model.load(m_device, filePath.str());
			return std::move(model);
		}

	private:

		RenderDevice * m_device;
	};
}
