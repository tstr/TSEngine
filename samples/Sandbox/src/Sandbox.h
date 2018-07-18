/*
	Sandbox application
*/

#pragma once

#include <tsengine.h>

#include "Camera.h"
#include "3D/ForwardRender.h"

#include "Entity.h"
#include "RenderComponent.h"
#include "TransformComponent.h"

namespace ts
{
	class Sandbox : public Application, private InputSystem::IListener
	{
	private:

		EntityManager m_entityManager;

		Camera m_camera;
		ForwardRenderer m_render;
		RenderTargets<> m_renderTarget;

		//Entities + Components
		std::vector<Entity> m_entities;
		ComponentMap<RenderComponent> m_renderables;
		ComponentMap<TransformComponent> m_transforms;

		Model m_sponzaModel;
		Model m_cubeModel;

		float m_scale;

		int onInit() override;
		void onExit() override;
		void onUpdate(double deltatime) override;

		void onKeyDown(EKeyCode code) override;

	public:

		Sandbox(int argc, char** argv);
		~Sandbox();

		EntityManager* getEntities() { return &m_entityManager; }

		int loadModel(Entity e, Model& model, const String& name);
	};
}
