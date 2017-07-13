/*
	Scene system header
*/

#pragma once

#include "Entity.h"
#include "SceneCamera.h"
#include "TransformComponent.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class Scene
	{
	private:
		
		EntityManager* m_entityManager;

		InputSystem* m_inputSystem;

		SceneCamera m_camera;

		ComponentMap<TransformComponent> m_transformComponents;

	public:

		Scene(EntityManager* entities, InputSystem* input);

		SceneCamera* getCamera() { return &m_camera; }

		EntityManager* getEntities() { return m_entityManager; }

		void setTransform(Entity e, Matrix t)
		{
			TransformComponent tcomp;
			tcomp.setMatrix(t);
			m_transformComponents.setComponent(e, TransformComponent{ t });
		}

		Matrix getTransform(Entity e)
		{
			TransformComponent comp;
			m_transformComponents.getComponent(e, comp);
			return comp.getMatrix();
		}
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
