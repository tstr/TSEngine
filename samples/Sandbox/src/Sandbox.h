/*
	Sandbox application
*/

#pragma once

#include <tsengine.h>

#include "3D/Graphics3D.h"
#include "Scene.h"
#include "Entity.h"

namespace ts
{
	class Sandbox : public Application, private InputSystem::IListener
	{
	private:

		EntityManager m_entityManager;

		Scene m_scene;

		Graphics3D m_g3D;

		
		std::vector<Entity> m_entities;
		float m_scale;


		int onInit() override;
		void onExit() override;
		void onUpdate(double deltatime) override;

		void onKeyDown(EKeyCode code) override;

	public:

		Sandbox(int argc, char** argv);
		~Sandbox();

		EntityManager* getEntities() { return &m_entityManager; }

		Scene* getScene() { return &m_scene; }

		int loadModel(Entity e, const String& name);
	};
}
