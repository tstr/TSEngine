/*
	Maths source
*/

#include "pch.h"
#include <C3E\Core\maths.h>

using namespace C3E;

////////////////////////////////////////////////////////////////////////////////////////

ALIGN(16) const Vector Vector::Zero = { 0, 0, 0, 0 };

ALIGN(16) const Matrix Matrix::Identity = Matrix(Internal::XMMatrixIdentity());
ALIGN(16) const Quaternion Quaternion::Identity = Quaternion(Internal::XMQuaternionIdentity());

////////////////////////////////////////////////////////////////////////////////////////