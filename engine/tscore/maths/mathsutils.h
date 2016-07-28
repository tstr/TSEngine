/*
	Shared utilities
*/

#pragma once

#include <tscore/system/memory.h>

#include <xmmintrin.h>
#include <DirectXMath.h>
#include <cmath>

#ifndef VECTOR_CALL
#define VECTOR_CALL __vectorcall
#endif

#ifdef VECTOR_NO_INLINE
#define VECTOR_INLINE
#else
#define VECTOR_INLINE __forceinline
#endif

namespace ts
{
	namespace internal
	{
		using namespace DirectX;
		typedef XMVECTOR SimdFloat;
		typedef XMMATRIX SimdMatrix;
	}

	class Matrix;
	class Quaternion;
	class Vector;

	VECTOR_INLINE bool VerifyCPUIntrinsicsSupport() { return internal::XMVerifyCPUSupport(); }
}