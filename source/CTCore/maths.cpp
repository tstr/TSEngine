/*
	Maths source
*/

#include "pch.h"
#include <CT\Core\maths.h>

using namespace CT;

////////////////////////////////////////////////////////////////////////////////////////

ALIGN(16) const Vector Vector::Zero = { 0, 0, 0, 0 };

ALIGN(16) const Matrix Matrix::Identity = Matrix(Internal::XMMatrixIdentity());
ALIGN(16) const Quaternion Quaternion::Identity = Quaternion(Internal::XMQuaternionIdentity());

////////////////////////////////////////////////////////////////////////////////////////