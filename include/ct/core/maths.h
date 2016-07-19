/*
	Abstraction layer for DirectXMath library

	Provides vector and matrix objects
*/

#pragma once

//Basic engine headers
#include <ct\core\memory.h>
#include <ct\internal\SimpleMath.h>

#include <ostream>
#include <cmath>

#include <xmmintrin.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#ifndef VECTOR_CALL
#define VECTOR_CALL __vectorcall
#endif

#ifdef VECTOR_NO_INLINE
#define VECTOR_INLINE
#else
#define VECTOR_INLINE __forceinline
#endif

namespace CT
{
	namespace Colours
	{
		using namespace DirectX::Colors;
	}

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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//DirectXTK helper classes
	using Plane = DirectX::SimpleMath::Plane;
	using Colour = DirectX::SimpleMath::Color;

	class Matrix;
	class Quaternion;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	namespace Internal
	{
		using namespace DirectX;
		typedef XMVECTOR SimdFloat;
		typedef XMMATRIX SimdMatrix;
	}

	VECTOR_INLINE bool VerifyCPUIntrinsicsSupport() { return Internal::XMVerifyCPUSupport(); }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//SIMD vector class - 4 32 bit components
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALIGN(16) class Vector :
		public Aligned<16>
	{
	protected:

		union
		{
			Internal::SimdFloat v128;
			float floats[4];
		};

	public:

		VECTOR_INLINE operator Internal::SimdFloat() const { return v128; }

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

		VECTOR_INLINE explicit Vector(Internal::SimdFloat v) { v128 = v; }

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
		VECTOR_INLINE bool VECTOR_CALL operator == (Vector v) const { return Internal::XMVector4Equal(v128, v.v128); }
		VECTOR_INLINE bool VECTOR_CALL operator != (Vector v) const { return !Internal::XMVector4Equal(v128, v.v128); };

		//Assignment operators
		VECTOR_INLINE Vector& VECTOR_CALL operator= (Vector V) { v128 = V.v128; return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator = (const Internal::SimdFloat& V) { v128 = V; return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator = (float v[4]) { memcpy(floats, v, 4 * sizeof(float)); return *this; }

		//Dot product
		VECTOR_INLINE static float VECTOR_CALL Dot(Vector v0, Vector v1) { return Vector(Internal::XMVector4Dot(v0.v128, v1.v128)).x(); }
		VECTOR_INLINE float VECTOR_CALL Dot(Vector v) const { return Vector::Dot(*this, v); };

		//Cross product
		VECTOR_INLINE static Vector VECTOR_CALL Cross(Vector v0, Vector v1) { return (Vector)Internal::XMVector3Cross(v0.v128, v1.v128); }
		VECTOR_INLINE Vector VECTOR_CALL Cross(const Vector& v) const { return Vector::Cross(*this, v); };

		VECTOR_INLINE float Length() const { return Vector(Internal::XMVector4Length(v128)).x(); }
		VECTOR_INLINE float VECTOR_CALL LengthSquared() const { return Vector(Internal::XMVector4LengthSq(v128)).x(); }

		VECTOR_INLINE void Normalize() { *this = Vector(Internal::XMVector4Normalize(v128)); }
		VECTOR_INLINE Vector Normalize() const { return Vector(Internal::XMVector4Normalize(v128)); }

		//Logical operations
		VECTOR_INLINE static Vector VECTOR_CALL Add(Vector v0, Vector v1) { return Vector(Internal::XMVectorAdd(v0.v128, v1.v128)); }
		VECTOR_INLINE static Vector VECTOR_CALL Subtract(Vector v0, Vector v1) { return Vector(Internal::XMVectorSubtract(v0.v128, v1.v128)); }
		VECTOR_INLINE static Vector VECTOR_CALL Multiply(Vector v0, Vector v1) { return Vector(Internal::XMVectorMultiply(v0.v128, v1.v128)); }
		VECTOR_INLINE static Vector VECTOR_CALL Divide(Vector v0, Vector v1) { return Vector(Internal::XMVectorDivide(v0.v128, v1.v128)); }

		VECTOR_INLINE static Vector VECTOR_CALL Scale(Vector v, float scalar) { return Vector(Internal::XMVectorScale(v.v128, scalar)); }

		VECTOR_INLINE static Vector VECTOR_CALL Transform(Vector v, const Matrix& m);
		VECTOR_INLINE static Vector VECTOR_CALL Transform(Vector v, const Quaternion& m);

		//Operator overloads
		VECTOR_INLINE Vector VECTOR_CALL operator+(Vector v) const { return Vector::Add(*this, v); }
		VECTOR_INLINE Vector VECTOR_CALL operator-(Vector v) const { return Vector::Subtract(*this, v); }
		VECTOR_INLINE Vector VECTOR_CALL operator*(Vector v) const { return Vector::Multiply(*this, v); }
		VECTOR_INLINE Vector VECTOR_CALL operator/(Vector v) const { return Vector::Divide(*this, v); }

		VECTOR_INLINE Vector& VECTOR_CALL operator+=(Vector v) { v128 = Vector::Add(*this, v); return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator-=(Vector v) { v128 = Vector::Subtract(*this, v); return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator*=(Vector v) { v128 = Vector::Multiply(*this, v); return *this; }
		VECTOR_INLINE Vector& VECTOR_CALL operator/=(Vector v) { v128 = Vector::Divide(*this, v); return *this; }

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

		VECTOR_INLINE Vector& operator*= (float scalar) { *this = Vector::Scale(*this, scalar); return *this; }
		VECTOR_INLINE Vector& operator/= (float scalar) { *this = Vector::Scale(*this, 1.0f / scalar); return *this; }

		//Urnary operators
		VECTOR_INLINE Vector operator+() const { return *this; }
		VECTOR_INLINE Vector operator-() const { return Vector(Internal::XMVectorNegate(v128)); }

		static const Vector Zero;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//4x4 Matrix class - uses Left Hand coordinate system
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALIGN(16) class Matrix :
		public Aligned<16>
	{
	public:

		union
		{
			struct { Internal::SimdMatrix m; };
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

		Matrix() : Matrix(Internal::XMMatrixIdentity()) {}

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
			m = Internal::XMMatrixIdentity();
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
		explicit Matrix(const Internal::SimdMatrix& mat) : m(mat) {}
		VECTOR_INLINE Matrix& VECTOR_CALL operator=(const Internal::SimdMatrix& mat) { m = mat; return *this; }

		operator Internal::SimdMatrix() const { return m; }

		//VECTOR_INLINE float* operator[](int index) { return mfloat2D[index]; }
		VECTOR_INLINE float& operator[](int index) { return mfloat[index]; }

		/////////////////////////////////////////////////////////////////////////////////////

		// Properties
		Vector VECTOR_CALL Up() const { return Vector(_21, _22, _23); }
		void Up(const Vector& v) { _21 = v.x(); _22 = v.y(); _23 = v.z(); }

		Vector VECTOR_CALL Down() const { return Vector(-_21, -_22, -_23); }
		void Down(const Vector& v) { _21 = -v.x(); _22 = -v.y(); _23 = -v.z(); }

		Vector VECTOR_CALL Right() const { return Vector(_11, _12, _13); }
		void Right(const Vector& v) { _11 = v.x(); _12 = v.y(); _13 = v.z(); }

		Vector VECTOR_CALL Left() const { return Vector(-_11, -_12, -_13); }
		void Left(const Vector& v) { _11 = -v.x(); _12 = -v.y(); _13 = -v.z(); }

		Vector VECTOR_CALL Forward() const  { return Vector(-_31, -_32, -_33); }
		void Forward(const Vector& v) { _31 = -v.x(); _32 = -v.y(); _33 = -v.z(); }

		Vector VECTOR_CALL Backward() const { return Vector(_31, _32, _33); }
		void Backward(const Vector& v) { _31 = v.x(); _32 = v.y(); _33 = v.z(); }

		Vector VECTOR_CALL Translation() const { return Vector(_41, _42, _43); }
		void Translation(const Vector& v) { _41 = v.x(); _42 = v.y(); _43 = v.z(); }

		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE Matrix VECTOR_CALL Transpose() const
		{
			return Matrix(Internal::XMMatrixTranspose(m));
		}

		VECTOR_INLINE void Transpose(const Matrix& mat)
		{
			m = (Internal::SimdMatrix)Internal::XMMatrixTranspose(mat.m);
		}

		VECTOR_INLINE Matrix VECTOR_CALL Inverse() const
		{
			Vector v;
			return Matrix(Internal::XMMatrixInverse((Internal::XMVECTOR*)&v, m));
		}

		VECTOR_INLINE bool VECTOR_CALL Decompose(_Out_ Vector& scale, _Out_ Quaternion& rotation, _Out_ Vector& translation);

		/////////////////////////////////////////////////////////////////////////////////////
		//Translation
		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE static Matrix VECTOR_CALL CreateTranslation(Vector position) { return Matrix(Internal::XMMatrixTranslationFromVector(position)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateTranslation(float x, float y, float z) { return Matrix(Internal::XMMatrixTranslation(x, y, z)); }

		VECTOR_INLINE static Matrix VECTOR_CALL CreateScale(Vector scales) { return Matrix(Internal::XMMatrixScalingFromVector(scales)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateScale(float xs, float ys, float zs) { return Matrix(Internal::XMMatrixScaling(xs, ys, zs)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateScale(float scale) { return Matrix(Internal::XMMatrixScaling(scale, scale, scale)); }

		//Left hand
		VECTOR_INLINE static Matrix VECTOR_CALL CreateLookAt(Vector position, Vector target, Vector up) { return Matrix(Internal::XMMatrixLookAtLH(position, target, up)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateLookTo(Vector position, Vector target, Vector up) { return Matrix(Internal::XMMatrixLookToLH(position, target, up)); }
		//VECTOR_INLINE static Matrix VECTOR_CALL CreateWorld(Vector position, Vector forward, Vector up) {}

		VECTOR_INLINE static Vector VECTOR_CALL Transform(Vector v, Matrix mat)
		{
			return Vector(Internal::XMVector3TransformCoord(v, mat));
		}

		/////////////////////////////////////////////////////////////////////////////////////
		//Rotation
		/////////////////////////////////////////////////////////////////////////////////////

		VECTOR_INLINE static Matrix VECTOR_CALL CreateRotationX(float radians) { return Matrix(Internal::XMMatrixRotationX(radians)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateRotationY(float radians) { return Matrix(Internal::XMMatrixRotationY(radians)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateRotationZ(float radians) { return Matrix(Internal::XMMatrixRotationZ(radians)); }

		VECTOR_INLINE static Matrix VECTOR_CALL CreateFromYawPitchRoll(Vector rot) { return Matrix(Internal::XMMatrixRotationRollPitchYawFromVector(rot)); }
		VECTOR_INLINE static Matrix VECTOR_CALL CreateFromYawPitchRoll(float pitch, float yaw, float roll) { return Matrix(Internal::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)); }

		VECTOR_INLINE static Matrix VECTOR_CALL CreateFromAxisAngle(Vector axis, float angle) { return Matrix(Internal::XMMatrixRotationAxis(axis, angle)); }

		VECTOR_INLINE static Matrix VECTOR_CALL CreateFromQuaternion(const Quaternion& quat);

		/////////////////////////////////////////////////////////////////////////////////////

		//Left handed coordinate system
		VECTOR_INLINE static Matrix VECTOR_CALL CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane)
		{
			return Matrix(Internal::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));
		}

		//Left handed coordinate system
		VECTOR_INLINE static Matrix VECTOR_CALL CreatePerspective(float width, float height, float nearPlane, float farPlane)
		{
			return Matrix(Internal::XMMatrixPerspectiveLH(width, height, nearPlane, farPlane));
		}

		//Left handed coordinate system
		VECTOR_INLINE static Matrix VECTOR_CALL CreateOrthographic(float width, float height, float nearPlane, float farPlane)
		{
			return Matrix(Internal::XMMatrixOrthographicLH(width, height, nearPlane, farPlane));
		}

		/////////////////////////////////////////////////////////////////////////////////////

		//Identity matrix
		static const Matrix Identity;
	};

	VECTOR_INLINE Matrix VECTOR_CALL operator/ (Matrix M1, Matrix M2) { return Matrix(Internal::XMMatrixMultiply(M1, M2.Inverse())); }
	VECTOR_INLINE Matrix VECTOR_CALL operator* (Matrix M1, Matrix M2) { return Matrix(Internal::XMMatrixMultiply(M1, M2)); }

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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Quaternion class
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALIGN(16) class Quaternion :
		public Aligned<16>
	{
	protected:

		union
		{
			Internal::SimdFloat v128;
			float floats[4];
		};

	public:

		//Constructors
		Quaternion() : Quaternion(Internal::XMQuaternionIdentity()) {}

		Quaternion(float _x, float _y, float _z, float _w)
		{
			floats[0] = _x;
			floats[1] = _y;
			floats[2] = _z;
			floats[3] = _w;
		}

		VECTOR_INLINE Quaternion(const Vector& v, float s) : v128(v) { floats[3] = s; }

		VECTOR_INLINE explicit Quaternion(Internal::SimdFloat f) : v128(f) {}
		VECTOR_INLINE explicit Quaternion(_In_reads_(4) const float *array) { memcpy(floats, array, 4 * sizeof(float)); }

		VECTOR_INLINE operator Internal::SimdFloat() const { return v128; }

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
		VECTOR_INLINE bool VECTOR_CALL operator== (Quaternion q) const { return Internal::XMVector4Equal(v128, q.v128); }
		VECTOR_INLINE bool VECTOR_CALL operator!= (Quaternion q) const { return !Internal::XMVector4Equal(v128, q.v128); }

		// Assignment operators
		VECTOR_INLINE Quaternion& VECTOR_CALL operator= (Quaternion q) { v128 = q.v128; return *this; }

		VECTOR_INLINE Quaternion& VECTOR_CALL operator += (Quaternion q) { v128 = Vector(v128) + Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator-= (Quaternion q) { v128 = Vector(v128) - Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator*= (Quaternion q) { v128 = Vector(v128) * Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator/= (Quaternion q) { v128 = Vector(v128) / Vector(q.v128); }
		VECTOR_INLINE Quaternion& VECTOR_CALL operator*= (float scalar) { v128 = Vector(v128) * scalar; }

		// Urnary operators
		VECTOR_INLINE Quaternion VECTOR_CALL operator+() const { return *this; }
		VECTOR_INLINE Quaternion VECTOR_CALL operator-() const { return Quaternion(Internal::XMVectorNegate(v128)); }

		VECTOR_INLINE Quaternion VECTOR_CALL Normalize() { return Quaternion(Internal::XMQuaternionNormalize(v128)); }
		VECTOR_INLINE void Normalize(_Out_ Quaternion& result) const { result = Quaternion(Internal::XMQuaternionNormalize(v128)); }

		VECTOR_INLINE Quaternion VECTOR_CALL Conjugate() const { return Quaternion(Internal::XMQuaternionConjugate(v128)); }
		VECTOR_INLINE void Conjugate(_Out_ Quaternion& result) const { result = Quaternion(Internal::XMQuaternionConjugate(v128)); }

		VECTOR_INLINE Quaternion VECTOR_CALL Inverse() const { return Quaternion(Internal::XMQuaternionInverse(v128)); }
		VECTOR_INLINE void Inverse(_Out_ Quaternion& result) const { result = Quaternion(Internal::XMQuaternionInverse(v128)); }

		VECTOR_INLINE float VECTOR_CALL Dot(Quaternion q) const
		{
			return Vector(Internal::XMQuaternionDot(v128, q.v128)).x();
		}

		// Static functions
		VECTOR_INLINE static Quaternion VECTOR_CALL CreateFromAxisAngle(Vector axis, float angle) { return Quaternion(Internal::XMQuaternionRotationAxis(axis, angle)); }
		VECTOR_INLINE static Quaternion VECTOR_CALL CreateFromYawPitchRoll(float pitch, float yaw, float roll) { return Quaternion(Internal::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll)); }
		VECTOR_INLINE static Quaternion VECTOR_CALL CreateFromYawPitchRoll(Vector v) { return Quaternion(Internal::XMQuaternionRotationRollPitchYawFromVector(v)); }
		VECTOR_INLINE static Quaternion VECTOR_CALL CreateFromRotationMatrix(Matrix m) { return Quaternion(Internal::XMQuaternionRotationMatrix(m)); }

		VECTOR_INLINE static Quaternion VECTOR_CALL Concatenate(Quaternion q1, Quaternion q2) { return Quaternion(Internal::XMQuaternionMultiply(q1, q2)); }

		// Constants
		static const Quaternion Identity;
	};

	VECTOR_INLINE Quaternion VECTOR_CALL operator+ (Quaternion Q1, Quaternion Q2) { return Q1 += Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator- (Quaternion Q1, Quaternion Q2) { return Q1 -= Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator* (Quaternion Q1, Quaternion Q2) { return Q1 *= Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator* (Quaternion Q, float S) { return Q *= S; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator/ (Quaternion Q1, Quaternion Q2) { return Q1 /= Q2; }
	VECTOR_INLINE Quaternion VECTOR_CALL operator* (float S, Quaternion Q) { return Q *= S; }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Vector definitions
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VECTOR_INLINE Vector VECTOR_CALL Vector::Transform(Vector v, const Matrix& m) { return Vector(Internal::XMVector4Transform(v, m)); }
	VECTOR_INLINE Vector VECTOR_CALL Vector::Transform(Vector v, const Quaternion& q) { return Vector(Internal::XMVector3Rotate(v, q)); }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Matrix definitions
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VECTOR_INLINE bool VECTOR_CALL Matrix::Decompose(_Out_ Vector& scale, _Out_ Quaternion& rotation, _Out_ Vector& translation)
	{
		Internal::SimdFloat s, r, t;
		if (!Internal::XMMatrixDecompose(&s, &r, &t, *this))
			return false;

		scale = Vector(s);
		rotation = Quaternion(r);
		translation = Vector(t);

		return true;
	}

	VECTOR_INLINE Matrix VECTOR_CALL Matrix::CreateFromQuaternion(const Quaternion& quat) { return Matrix(Internal::XMMatrixRotationQuaternion(quat)); }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	VECTOR_INLINE std::ostream& VECTOR_CALL operator<<(std::ostream& stream, Vector vec)
	{
		stream << vec.x() << ","
			<< vec.y() << ","
			<< vec.z() << ","
			<< vec.w();

		return stream;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}