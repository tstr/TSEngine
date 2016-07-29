/*
	SIMD Quaternion class - 32 bit components
*/

#pragma once

//Basic engine headers
#include "common.h"
#include "vector.h"

namespace ts
{
	ALIGN(16) class Quaternion :
		public Aligned<16>
	{
	protected:

		union
		{
			internal::SimdFloat v128;
			float floats[4];
		};

	public:

		//Constructors
		Quaternion() : Quaternion(internal::XMQuaternionIdentity()) {}

		Quaternion(float _x, float _y, float _z, float _w)
		{
			floats[0] = _x;
			floats[1] = _y;
			floats[2] = _z;
			floats[3] = _w;
		}

		VECTOR_INLINE Quaternion(const Vector& v, float s) : v128(v) { floats[3] = s; }

		VECTOR_INLINE explicit Quaternion(internal::SimdFloat f) : v128(f) {}
		VECTOR_INLINE explicit Quaternion(_In_reads_(4) const float *array) { memcpy(floats, array, 4 * sizeof(float)); }

		VECTOR_INLINE operator internal::SimdFloat() const { return v128; }

		//Accessors
		VECTOR_INLINE float x() const { return floats[0]; }
		VECTOR_INLINE float y() const { return floats[1]; }
		VECTOR_INLINE float z() const { return floats[2]; }
		VECTOR_INLINE float w() const { return floats[3]; }

		VECTOR_INLINE float& x() { return floats[0]; }
		VECTOR_INLINE float& y() { return floats[1]; }
		VECTOR_INLINE float& z() { return floats[2]; }
		VECTOR_INLINE float& w() { return floats[3]; }

		// Comparision operators
		VECTOR_INLINE bool VECTOR_CALL operator== (Quaternion q) const { return internal::XMVector4Equal(v128, q.v128); }
		VECTOR_INLINE bool VECTOR_CALL operator!= (Quaternion q) const { return !internal::XMVector4Equal(v128, q.v128); }

		// Assignment operators
		VECTOR_INLINE Quaternion& VECTOR_CALL operator= (Quaternion q) { v128 = q.v128; return *this; }

		VECTOR_INLINE Quaternion& VECTOR_CALL operator += (Quaternion q) { v128 = Vector(v128) + Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator-= (Quaternion q) { v128 = Vector(v128) - Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator*= (Quaternion q) { v128 = Vector(v128) * Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator/= (Quaternion q) { v128 = Vector(v128) / Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator*= (float scalar) { v128 = Vector(v128) * scalar; }

		// Urnary operators
		VECTOR_INLINE Quaternion VECTOR_CALL operator+() const { return *this; }
		VECTOR_INLINE Quaternion VECTOR_CALL operator-() const { return Quaternion(internal::XMVectorNegate(v128)); }

		VECTOR_INLINE Quaternion VECTOR_CALL normalize() { return Quaternion(internal::XMQuaternionNormalize(v128)); }
		VECTOR_INLINE void normalize(_Out_ Quaternion& result) const { result = Quaternion(internal::XMQuaternionNormalize(v128)); }

		VECTOR_INLINE Quaternion VECTOR_CALL conjugate() const { return Quaternion(internal::XMQuaternionConjugate(v128)); }
		VECTOR_INLINE void conjugate(_Out_ Quaternion& result) const { result = Quaternion(internal::XMQuaternionConjugate(v128)); }

		VECTOR_INLINE Quaternion VECTOR_CALL inverse() const { return Quaternion(internal::XMQuaternionInverse(v128)); }
		VECTOR_INLINE void inverse(_Out_ Quaternion& result) const { result = Quaternion(internal::XMQuaternionInverse(v128)); }

		VECTOR_INLINE float VECTOR_CALL dot(Quaternion q) const
		{
			return Vector(internal::XMQuaternionDot(v128, q.v128)).x();
		}

		// Static functions
		VECTOR_INLINE static Quaternion VECTOR_CALL fromAxisAngle(Vector axis, float angle) { return Quaternion(internal::XMQuaternionRotationAxis(axis, angle)); }
		VECTOR_INLINE static Quaternion VECTOR_CALL fromYawPitchRoll(float pitch, float yaw, float roll) { return Quaternion(internal::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll)); }
		VECTOR_INLINE static Quaternion VECTOR_CALL fromYawPitchRoll(Vector v) { return Quaternion(internal::XMQuaternionRotationRollPitchYawFromVector(v)); }
		//VECTOR_INLINE static Quaternion VECTOR_CALL CreateFromRotationMatrix(Matrix m) { return Quaternion(internal::XMQuaternionRotationMatrix((internal::SimdMatrix&)m)); }

		VECTOR_INLINE static Quaternion VECTOR_CALL concatenate(Quaternion q1, Quaternion q2) { return Quaternion(internal::XMQuaternionMultiply(q1, q2)); }

		VECTOR_INLINE static Vector VECTOR_CALL transform(Vector v, Quaternion q) { return internal::XMVector3Rotate(v, q); }
		
		// Constants
		static const Quaternion Identity;
	};
	
	VECTOR_INLINE Quaternion VECTOR_CALL operator+ (Quaternion Q1, Quaternion Q2) { return Q1 += Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator- (Quaternion Q1, Quaternion Q2) { return Q1 -= Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator* (Quaternion Q1, Quaternion Q2) { return Q1 *= Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator* (Quaternion Q, float S) { return Q *= S; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator/ (Quaternion Q1, Quaternion Q2) { return Q1 /= Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator* (float S, Quaternion Q) { return Q *= S; }
}