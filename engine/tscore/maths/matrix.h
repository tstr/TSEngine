/*
	4x4 Matrix class - uses Left Hand coordinate system
*/

#pragma once

//Basic engine headers
#include "common.h"
#include "vector.h"
#include "quaternion.h"

namespace ts
{
	ALIGN(16) class Matrix :
		public Aligned<16>
	{
	public:

		union
		{
			struct { internal::SimdMatrix m; };
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			struct { float mfloat2D[4][4]; };
			struct { float mfloat[4 * 4]; };
		};

		/////////////////////////////////////////////////////////////////////////////////////
		//ctor
		/////////////////////////////////////////////////////////////////////////////////////

		Matrix() : Matrix(internal::XMMatrixIdentity()) {}

		Matrix(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33) :
			m(m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33)
		{}

		Matrix(const Vector& r0, const Vector& r1, const Vector& r2)
		{
			m = internal::XMMatrixIdentity();
			m.r[0] = r0;
			m.r[1] = r1;
			m.r[2] = r2;
		}

		Matrix(const Vector& r0, const Vector& r1, const Vector& r2, const Vector& r3)
		{
			m.r[0] = r0;
			m.r[1] = r1;
			m.r[2] = r2;
			m.r[3] = r3;
		}

		Matrix& operator=(const Matrix& M)
		{
			m.r[0] = M.m.r[0];
			m.r[1] = M.m.r[1];
			m.r[2] = M.m.r[2];
			m.r[3] = M.m.r[3];
			return *this;
		}

		explicit Matrix(_In_reads_(16) const float* _floats) { memcpy(&m, _floats, 16 * sizeof(float)); }
		explicit Matrix(const internal::SimdMatrix& mat) : m(mat) {}
		VECTOR_INLINE Matrix& VECTOR_CALL operator=(const internal::SimdMatrix& mat) { m = mat; return *this; }

		operator internal::SimdMatrix() const { return this->m; }

		//VECTOR_INLINE float* operator[](int index) { return mfloat2D[index]; }
		//VECTOR_INLINE float& operator[](int index) { return mfloat[index]; }

		float getElement(uint32 row, uint32 column) const
		{
			//tsassert(row <= 4 && column <= 4);
			return mfloat2D[row][column];
		}

		void setElement(uint32 row, uint32 column, float el)
		{
			//tsassert(row <= 4 && column <= 4);
			mfloat2D[row][column] = el;
		}

		/////////////////////////////////////////////////////////////////////////////////////

		// Properties
		Vector VECTOR_CALL getUp() const { return Vector(_21, _22, _23); }
		void setUp(const Vector& v) { _21 = v.x(); _22 = v.y(); _23 = v.z(); }

		Vector VECTOR_CALL getDown() const { return Vector(-_21, -_22, -_23); }
		void setDown(const Vector& v) { _21 = -v.x(); _22 = -v.y(); _23 = -v.z(); }

		Vector VECTOR_CALL getRight() const { return Vector(_11, _12, _13); }
		void setRight(const Vector& v) { _11 = v.x(); _12 = v.y(); _13 = v.z(); }

		Vector VECTOR_CALL getLeft() const { return Vector(-_11, -_12, -_13); }
		void setLeft(const Vector& v) { _11 = -v.x(); _12 = -v.y(); _13 = -v.z(); }

		Vector VECTOR_CALL getForward() const  { return Vector(-_31, -_32, -_33); }
		void setForward(const Vector& v) { _31 = -v.x(); _32 = -v.y(); _33 = -v.z(); }

		Vector VECTOR_CALL getBackward() const { return Vector(_31, _32, _33); }
		void setBackward(const Vector& v) { _31 = v.x(); _32 = v.y(); _33 = v.z(); }

		Vector VECTOR_CALL getTranslation() const { return Vector(_41, _42, _43); }
		void setTranslation(const Vector& v) { _41 = v.x(); _42 = v.y(); _43 = v.z(); }

		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE Matrix VECTOR_CALL transpose() const
		{
			return Matrix(internal::XMMatrixTranspose(m));
		}

		VECTOR_INLINE Matrix VECTOR_CALL inverse() const
		{
			Vector v;
			return Matrix(internal::XMMatrixInverse((internal::XMVECTOR*)&v, m));
		}

		static VECTOR_INLINE void VECTOR_CALL inverse(Matrix& m)
		{
			m = m.inverse();
		}

		static VECTOR_INLINE void VECTOR_CALL transpose(Matrix& m)
		{
			m = m.transpose();
		}

		VECTOR_INLINE bool VECTOR_CALL decompose(_Out_ Vector& scale, _Out_ Quaternion& rotation, _Out_ Vector& translation);

		/////////////////////////////////////////////////////////////////////////////////////
		//Translation
		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE static Matrix VECTOR_CALL translation(Vector position) { return Matrix(internal::XMMatrixTranslationFromVector(position)); }
		VECTOR_INLINE static Matrix VECTOR_CALL translation(float x, float y, float z) { return Matrix(internal::XMMatrixTranslation(x, y, z)); }

		VECTOR_INLINE static Matrix VECTOR_CALL scale(Vector scales) { return Matrix(internal::XMMatrixScalingFromVector(scales)); }
		VECTOR_INLINE static Matrix VECTOR_CALL scale(float xs, float ys, float zs) { return Matrix(internal::XMMatrixScaling(xs, ys, zs)); }
		VECTOR_INLINE static Matrix VECTOR_CALL scale(float scale) { return Matrix(internal::XMMatrixScaling(scale, scale, scale)); }

		//Left hand
		VECTOR_INLINE static Matrix VECTOR_CALL lookAt(Vector position, Vector target, Vector up) { return Matrix(internal::XMMatrixLookAtLH(position, target, up)); }
		VECTOR_INLINE static Matrix VECTOR_CALL lookTo(Vector position, Vector target, Vector up) { return Matrix(internal::XMMatrixLookToLH(position, target, up)); }
		//VECTOR_INLINE static Matrix VECTOR_CALL CreateWorld(Vector position, Vector forward, Vector up) {}
		
		/////////////////////////////////////////////////////////////////////////////////////
		//Rotation
		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE static Matrix VECTOR_CALL rotationX(float radians) { return Matrix(internal::XMMatrixRotationX(radians)); }
		VECTOR_INLINE static Matrix VECTOR_CALL rotationY(float radians) { return Matrix(internal::XMMatrixRotationY(radians)); }
		VECTOR_INLINE static Matrix VECTOR_CALL rotationZ(float radians) { return Matrix(internal::XMMatrixRotationZ(radians)); }

		VECTOR_INLINE static Matrix VECTOR_CALL fromYawPitchRoll(Vector rot) { return Matrix(internal::XMMatrixRotationRollPitchYawFromVector(rot)); }
		VECTOR_INLINE static Matrix VECTOR_CALL fromYawPitchRoll(float pitch, float yaw, float roll) { return Matrix(internal::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)); }

		VECTOR_INLINE static Matrix VECTOR_CALL fromAxisAngle(Vector axis, float angle) { return Matrix(internal::XMMatrixRotationAxis(axis, angle)); }

		VECTOR_INLINE static Matrix VECTOR_CALL fromQuaternion(const Quaternion& quat);

		/////////////////////////////////////////////////////////////////////////////////////
		
		//Left handed coordinate system
		VECTOR_INLINE static Matrix VECTOR_CALL perspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane)
		{
			return Matrix(internal::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));
		}

		//Left handed coordinate system
		VECTOR_INLINE static Matrix VECTOR_CALL perspective(float width, float height, float nearPlane, float farPlane)
		{
			return Matrix(internal::XMMatrixPerspectiveLH(width, height, nearPlane, farPlane));
		}

		//Left handed coordinate system
		VECTOR_INLINE static Matrix VECTOR_CALL orthographic(float width, float height, float nearPlane, float farPlane)
		{
			return Matrix(internal::XMMatrixOrthographicLH(width, height, nearPlane, farPlane));
		}

		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE static Vector VECTOR_CALL transform(Vector v, Matrix q) { return internal::XMVector4Transform(v, q); }

		//Identity matrix
		static Matrix identity()
		{
			return Matrix(internal::XMMatrixIdentity());
		}
	};
	
	
	VECTOR_INLINE Matrix VECTOR_CALL operator/ (Matrix M1, Matrix M2) { return Matrix(internal::XMMatrixMultiply(M1, M2.inverse())); }
	VECTOR_INLINE Matrix VECTOR_CALL operator* (Matrix M1, Matrix M2) { return Matrix(internal::XMMatrixMultiply(M1, M2)); }

	VECTOR_INLINE Matrix VECTOR_CALL operator*(Matrix mat, float scalar)
	{
		mat.m.r[0] = Vector(mat.m.r[0]) * scalar;
		mat.m.r[1] = Vector(mat.m.r[1]) * scalar;
		mat.m.r[2] = Vector(mat.m.r[2]) * scalar;
		mat.m.r[3] = Vector(mat.m.r[3]) * scalar;
		return mat;
	}

	VECTOR_INLINE Matrix VECTOR_CALL operator/(Matrix mat, float _scalar)
	{
		float scalar = 1.0f / _scalar;
		mat.m.r[0] = Vector(mat.m.r[0]) * scalar;
		mat.m.r[1] = Vector(mat.m.r[1]) * scalar;
		mat.m.r[2] = Vector(mat.m.r[2]) * scalar;
		mat.m.r[3] = Vector(mat.m.r[3]) * scalar;
		return mat;
	}
	
	VECTOR_INLINE bool VECTOR_CALL Matrix::decompose(_Out_ Vector& scale, _Out_ Quaternion& rotation, _Out_ Vector& translation)
	{
		internal::SimdFloat s, r, t;
		if (!internal::XMMatrixDecompose(&s, &r, &t, *this))
			return false;

		scale = Vector(s);
		rotation = Quaternion(r);
		translation = Vector(t);

		return true;
	}

	VECTOR_INLINE Matrix VECTOR_CALL Matrix::fromQuaternion(const Quaternion& quat) { return Matrix(internal::XMMatrixRotationQuaternion((const internal::SimdFloat&)quat)); }
}