/*
	Transform component
*/

#pragma once

#include <tscore/maths.h>

namespace ts
{
	class TransformComponent
	{
	private:

		Matrix m_t;

	public:

		TransformComponent() {}

		TransformComponent(Matrix t) :
			m_t(t)
		{}

		TransformComponent(Vector position, Quaternion rotation = Quaternion()) :
			m_t(Matrix::fromQuaternion(rotation) * Matrix::translation(position))
		{}

		Matrix getMatrix() const { return m_t; }
		void setMatrix(Matrix t) { m_t = t; }

		void setPosition(Vector p) { m_t.setTranslation(p); }
	};
}
