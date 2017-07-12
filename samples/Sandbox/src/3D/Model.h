/*
	Model Importer class
*/

#pragma once

#include <tscore/path.h>
#include <tscore/ptr.h>
#include <tsgraphics/model/modeldefs.h>
#include <tsgraphics/GraphicsContext.h>

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
	};

	class CModel
	{
	public:

		struct Selection
		{
			SMesh submesh;
			SMaterial material;
		};

		typedef std::vector<Selection> SelectionList;
		typedef SelectionList::const_iterator SelectionIterator;

		CModel(GraphicsContext* graphics);
		~CModel() {}

		CModel(const CModel&) = delete;
		CModel(CModel&&) = default;

		bool import(const Path& path, uint8 attribMask = 0xff);

		SelectionIterator beginSection() const { return m_selections.begin(); }
		SelectionIterator endSection() const { return m_selections.end(); }

		SelectionList::size_type sectionCount() const { return m_selections.size(); }

		MeshId getMeshID() const { return m_modelMesh; }

	private:

		GraphicsContext* m_graphics;

		Path m_filepath;

		MeshId m_modelMesh;
		SelectionList m_selections;
	};
}