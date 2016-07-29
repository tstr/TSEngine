/*
	SIMD vector class - 4 32 bit components
*/

#pragma once

//Basic engine headers
#include "common.h"

namespace ts
{
	class Matrix;
	class Quaternion;

	ALIGN(16) class Vector :
		public Aligned<16>
	{
	protected:

		union
		{
			internal::SimdFloat v128;
			float floats[4];
		};

	public:

		VECTOR_INLINE operator internal::SimdFloat() const { return v128; }

		//constructors

		VECTOR_INLINE Vector()
		{
			floats[0] = 0;
			floats[1] = 0;
			floats[2] = 0;
			floats[3] = 0;
		}

		VECTOR_INLINE Vector(float x, float y, float z = 0.0f, float w = 0.0f)
		{
			floats[0] = x;
			floats[1] = y;
			floats[2] = z;
			floats[3] = w;
		}

		VECTOR_INLINE explicit Vector(_In_reads_(4) const float* f)
		{
			memcpy(floats, f, 4 * sizeof(float));
		}

		VECTOR_INLINE Vector(internal::SimdFloat v) { v128 = v; }

		//accessors
		VECTOR_INLINE float x() const { return floats[0]; }
		VECTOR_INLINE float y() const { return floats[1]; }
		VECTOR_INLINE float z() const { return floats[2]; }
		VECTOR_INLINE float w() const { return floats[3]; }

		VECTOR_INLINE float& x() { return floats[0]; }
		VECTOR_INLINE float& y() { return floats[1]; }
		VECTOR_INLINE float& z() { return floats[2]; }
		VECTOR_INLINE float& w() { return floats[3]; }

		//Comparision operators
		VECTOR_INLINE bool VECTOR_CALL operator == (Vector v) const { return internal::XMVector4Equal(v128, v.v128); }
		VECTOR_INLINE bool VECTOR_CALL operator != (Vector v) const { return !internal::XMVector4Equal(v128, v.v128); };

		//Assignment operators
		VECTOR_INLINE Vector& VECTOR_CALL operator= (Vector V) { v128 = V.v128; return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator = (const internal::SimdFloat& V) { v128 = V; return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator = (float v[4]) { memcpy(floats, v, 4 * sizeof(float)); return *this; }

		//Dot product
		VECTOR_INLINE static float VECTOR_CALL dot(Vector v0, Vector v1) { return Vector(internal::XMVector4Dot(v0.v128, v1.v128)).x(); }
		VECTOR_INLINE float VECTOR_CALL dot(Vector v) const { return Vector::dot(*this, v); };

		//Cross product
		VECTOR_INLINE static Vector VECTOR_CALL cross(Vector v0, Vector v1) { return (Vector)internal::XMVector3Cross(v0.v128, v1.v128); }
		VECTOR_INLINE Vector VECTOR_CALL cross(const Vector& v) const { return Vector::cross(*this, v); };

		VECTOR_INLINE float length() const { return Vector(internal::XMVector4Length(v128)).x(); }
		VECTOR_INLINE float VECTOR_CALL lengthSquared() const { return Vector(internal::XMVector4LengthSq(v128)).x(); }

		VECTOR_INLINE void normalize() { *this = Vector(internal::XMVector4Normalize(v128)); }
		VECTOR_INLINE Vector normalize() const { return Vector(internal::XMVector4Normalize(v128)); }

		//Logical operations
		VECTOR_INLINE static Vector VECTOR_CALL add(Vector v0, Vector v1) { return Vector(internal::XMVectorAdd(v0.v128, v1.v128)); }
		VECTOR_INLINE static Vector VECTOR_CALL subtract(Vector v0, Vector v1) { return Vector(internal::XMVectorSubtract(v0.v128, v1.v128)); }
		VECTOR_INLINE static Vector VECTOR_CALL multiply(Vector v0, Vector v1) { return Vector(internal::XMVectorMultiply(v0.v128, v1.v128)); }
		VECTOR_INLINE static Vector VECTOR_CALL divide(Vector v0, Vector v1) { return Vector(internal::XMVectorDivide(v0.v128, v1.v128)); }

		VECTOR_INLINE static Vector VECTOR_CALL scale(Vector v, float scalar) { return Vector(internal::XMVectorScale(v.v128, scalar)); }

		//VECTOR_INLINE static Vector VECTOR_CALL transform(Vector v, const Matrix& m);
		//VECTOR_INLINE static Vector VECTOR_CALL transform(Vector v, const Quaternion& m);

		//Operator overloads
		VECTOR_INLINE Vector VECTOR_CALL operator+(Vector v) const { return Vector::add(*this, v); }
		VECTOR_INLINE Vector VECTOR_CALL operator-(Vector v) const { return Vector::subtract(*this, v); }
		VECTOR_INLINE Vector VECTOR_CALL operator*(Vector v) const { return Vector::multiply(*this, v); }
		VECTOR_INLINE Vector VECTOR_CALL operator/(Vector v) const { return Vector::divide(*this, v); }

		VECTOR_INLINE Vector& VECTOR_CALL operator+=(Vector v) { v128 = Vector::add(*this, v); return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator-=(Vector v) { v128 = Vector::subtract(*this, v); return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator*=(Vector v) { v128 = Vector::multiply(*this, v); return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator/=(Vector v) { v128 = Vector::divide(*this, v); return *this; }

		//Scalar logical operations
		VECTOR_INLINE Vector VECTOR_CALL operator*(float s) const
		{
			Vector v = *this;
			v.x() *= s;
			v.y() *= s;
			v.z() *= s;
			v.w() *= s;
			return v;
		}
		VECTOR_INLINE Vector VECTOR_CALL operator/(float s) const
		{
			Vector v = *this;
			v.x() /= s;
			v.y() /= s;
			v.z() /= s;
			v.w() /= s;
			return v;
		}

		VECTOR_INLINE Vector& operator*= (float scalar) { *this = Vector::scale(*this, scalar); return *this; }
		VECTOR_INLINE Vector& operator/= (float scalar) { *this = Vector::scale(*this, 1.0f / scalar); return *this; }

		//Urnary operators
		VECTOR_INLINE Vector operator+() const { return *this; }
		VECTOR_INLINE Vector operator-() const { return Vector(internal::XMVectorNegate(v128)); }

		static const Vector Zero;
	};
}