/*
	Model classes
*/

#pragma once

#include <tsgraphics/rendermodule.h>
#include <tscore/filesystem/path.h>
#include <tsgraphics/model/modeldefs.h>


namespace ts
{
	struct SMaterial
	{
		struct SParams
		{
			Vector diffuseColour;
			Vector ambientColour;
			Vector specularColour;
			Vector emissiveColour;
			float specularPower = 0.0f;
			
			uint useDiffuseMap = 0;
			uint useNormalMap = 0;
			uint useDisplacementMap = 0;
			uint useSpecularMap = 0;
		} params;

		//Textures
		CTexture2D diffuseMap;
		CTexture2D normalMap;
		CTexture2D specularMap;
		CTexture2D displacementMap;
		CTexture2D ambientMap;
	};
	
	struct SMesh
	{
		Index indexOffset = 0;
		Index indexCount = 0;
		Index vertexBase = 0;

		uint8 vertexAttributes = 0;
		
		SMaterial material;
	};
	
	class CModel
	{
	private:
	
		CRenderModule* m_rendermodule = nullptr;
		
		Path m_filepath;
		
		std::vector<SMesh> m_meshes;
		
		CVertexBuffer m_vertices;
		CIndexBuffer m_indices;
		
	public:
		
		CModel(CRenderModule* module, const Path& path);
		~CModel() {}
		
		CModel(const CModel&) = delete;
		CModel(CModel&&) = default;
		
		bool import(const Path& path);
		
		uint32 getMeshCount() const { return (uint32)m_meshes.size(); }
		const SMesh& getMesh(uint32 idx) const { return m_meshes.at(idx); }
		
		void getVertexBuffer(CVertexBuffer& vbuffer) const { vbuffer = m_vertices; }
		void getIndexBuffer(CIndexBuffer& ibuffer) const { ibuffer = m_indices; }
	};
}
