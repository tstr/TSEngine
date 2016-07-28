/*
	Basic mathematical functions and constants
*/

#pragma once

#include "mathsutils.h"

namespace ts
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//trigonometry
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	using std::sin;	using std::asin;
	using std::cos;	using std::acos;
	using std::tan;	using std::atan;

	const float Pi = 3.14159265358979323846264f;

	VECTOR_INLINE float toRadians(float deg) { return (deg * (Pi / 180.0f)); }
	VECTOR_INLINE double toRadians(double deg) { return (deg * ((double)Pi / 180.0)); }

	VECTOR_INLINE float toDegrees(float rad) { return (rad * (180.0f / Pi)); }
	VECTOR_INLINE double toDegrees(double rad) { return (rad * (180.0 / (double)Pi)); }
}