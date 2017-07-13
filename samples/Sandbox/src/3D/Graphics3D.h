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

	struct LightSourceComponent
	{
		Vector colour;

		float constant;
		float linear;
		float quadratic;
	};

	/*
		3D renderer
	*/
	class Graphics3D
	{
	private:

		Scene* m_scene;

		GraphicsContext m_context;
		ColourPass m_passColour;

		Matrix m_matrixView;
		Matrix m_matrixProj;

		HBuffer m_constScene;
		HBuffer m_constMesh;

		ComponentMap<GraphicsComponent> m_graphicsComponents;
		ComponentMap<LightSourceComponent> m_lightSourceComponents;

	public:
		
		/*
			Render Item ID
		*/
		typedef GraphicsContext::ItemID Item;
		
		Graphics3D(GraphicsSystem* system, Scene* scene);
		~Graphics3D();

		GraphicsContext* getContext() { return &m_context; }

		void createGraphicsComponent(Entity e, MeshId mesh, const SubmeshInfo* info, size_t infoCount);
		void createLightComponent(Entity e, Vector colour, float constant, float linear, float quadratic);

		void submit(Entity e);

		void update();
	};
}
