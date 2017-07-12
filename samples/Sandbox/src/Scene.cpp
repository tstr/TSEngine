/*
	Scene system source
*/

#include "Scene.h"

#include <tscore/debug/assert.h>

using namespace ts;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

Scene::Scene(EntityManager* entities, InputSystem* input) :
	m_entityManager(entities),
	m_inputSystem(input),
	m_camera(m_inputSystem),
	m_transformComponents(m_entityManager)
{
	tsassert(m_inputSystem);
	tsassert(m_entityManager);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////