/*
	Graphics model loading API
*/

#pragma once


#include <C3E\core\corecommon.h>
#include <C3E\core\maths.h>
#include <C3E\core\strings.h>
#include <C3E\gfx\graphics.h>

#define MESH_STRING_LENGTH 32
#define MATERIAL_STRING_LENGTH 32
#define MATERIAL_TEXTURES_COUNT 8
#define MATERIAL_TEXTURE_PATH MAX_PATH

namespace C3E
{
	static const char g_material_extension[] = "cmat";
	static const char g_model_extension[] = "c3d";

	struct BoundingBox
	{
		Vector max;
		Vector min;
	};

	struct BoundingSphere
	{
		Vector centre;
		float radius;
	};

	struct ModelMesh
	{
		char nameMesh[MESH_STRING_LENGTH];
		char nameMaterial[MATERIAL_STRING_LENGTH];

		uint32 indexStart = 0;
		uint32 indexCount = 0;
		uint32 vertexAttributeMask = 0;

		BoundingBox bounds;
	};

	struct ModelMaterial
	{
		char name[MATERIAL_STRING_LENGTH];

		Vector cDiffuse;
		Vector cAmbient;
		Vector cSpecular;
		Vector cEmissive;
		float shininess;
		float alpha;

		char texDiffuse[MATERIAL_TEXTURE_PATH];
		char texNormal[MATERIAL_TEXTURE_PATH];
		char texSpecular[MATERIAL_TEXTURE_PATH];
		char texDisplacement[MATERIAL_TEXTURE_PATH];
		char texAmbient[MATERIAL_TEXTURE_PATH];
	};

	class C3E_GFX_API ModelExporter
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

	public:

		ModelExporter();
		~ModelExporter();

		void AddMesh(
			const char* name,
			const Vertex* vertices,
			const Index* indices,
			uint32 vnum,
			uint32 inum,
			uint32 attributeMask,
			const char* material
		);

		void AddMesh(
			const char* name,
			const std::vector<Vertex>& vertices,
			const std::vector<Index>& indices,
			uint32 attributeMask,
			const char* material
		)
		{
			AddMesh(name, &vertices[0], &indices[0], (uint32)vertices.size(), (uint32)indices.size(), attributeMask, material);
		}

		void AddMaterial(const ModelMaterial& mat);
		void SetScale(float scale);

		bool Save(const char* dest);
	};

	class C3E_GFX_API ModelImporter
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

	public:

		ModelImporter(const char* model);
		~ModelImporter();

		bool GetMaterial(const char* name, ModelMaterial& mat) const;
		bool GetMaterial(uint32 index, ModelMaterial& mat) const;
		bool GetMesh(const char* name, ModelMesh& mesh) const;
		bool GetMesh(uint32 index, ModelMesh& mesh) const;

		uint32 GetMeshCount() const;
		uint32 GetMaterialCount() const;

		const Vertex* GetVertices() const;
		const Index* GetIndices() const;
		uint32 GetIndexCount() const;
		uint32 GetVertexCount() const;

		void GetBoundingBox(BoundingBox& box) const;
		std::string GetDirectory() const;
		float GetScale() const;
	};
}