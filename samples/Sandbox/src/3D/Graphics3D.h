/*
	3D Graphics class
*/

#pragma once

#include "../Entity.h"
#include "../Scene.h"

#include <tsgraphics/GraphicsContext.h>
#include <tscore/pathutil.h>

#include "Model.h"
#include "ColourPass.h"
#include "MaterialInfo.h"

namespace ts
{
	class Sandbox;

	struct SubmeshInfo
	{
		MaterialStates states;

		MaterialResources resources;

		SMesh submeshView;
	};

	struct GraphicsComponent
	{
		std::vector<GraphicsContext::ItemID> items;
	};

	/*
		3D renderer
	*/
	class Graphics3D : public GraphicsContext
	{
	private:

		Scene* m_scene;

		ColourPass m_passColour;

		Matrix m_matrixView;
		Matrix m_matrixProj;

		HBuffer m_constScene;
		HBuffer m_constMesh;

		ComponentManager<GraphicsComponent> m_graphicsComponents;

	public:
		
		/*
			Render Item ID
		*/
		typedef GraphicsContext::ItemID Item;
		
		Graphics3D(GraphicsSystem* system, Scene* scene);
		~Graphics3D();

		void createComponent(Entity e, MeshId mesh, const SubmeshInfo* info, size_t infoCount);
		void submit(Entity e);

		void update();
	};
}
