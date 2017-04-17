/*
	Model Importer class
*/

#pragma once

#include <tsgraphics/GraphicsContext.h>
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
		TextureId diffuseMap;
		TextureId normalMap;
		TextureId specularMap;
		TextureId displacementMap;
		TextureId ambientMap;
	};

	struct SMesh
	{
		Index indexOffset = 0;
		Index indexCount = 0;
		Index vertexBase = 0;

		uint8 vertexAttributes = 0;

		MeshId id;

		SMaterial material;
	};

	class CModel
	{
	private:

		GraphicsContext* m_graphics = nullptr;

		Path m_filepath;

		std::vector<SMesh> m_meshes;

		void saveMeshInstance(const std::vector<SModelVertex>& vertices, const std::vector<ModelIndex>& indices, SMesh& meshDesc, uint8 attribMask);

	public:

		CModel(GraphicsContext* graphics);
		~CModel() {}

		CModel(const CModel&) = delete;
		CModel(CModel&&) = default;

		bool import(const Path& path, uint8 attribMask = 0xff);

		uint32 getMeshCount() const { return (uint32)m_meshes.size(); }
		const SMesh& getMesh(uint32 idx) const { return m_meshes.at(idx); }
	};
}