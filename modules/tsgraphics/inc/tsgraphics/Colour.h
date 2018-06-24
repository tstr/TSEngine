/*
	Colours
*/

#pragma once

#include <DirectXColors.h>
#include <tscore/maths.h>

namespace ts
{
	namespace colours
	{
		using namespace DirectX::Colors;
	}

	class RGBA
	{
	private:

		union
		{
			struct
			{
				byte r;
				byte g;
				byte b;
				byte a;
			};

			uint32 data;
		};

	public:

		RGBA() : data(0) {}

		RGBA(uint32 d) : data(d) {}

		RGBA(byte red, byte green, byte blue, byte alpha = 255) :
			r(red),
			g(green),
			b(blue),
			a(alpha)
		{}

		RGBA(Vector v)
		{
			r = (byte)(255.0f * v.x());
			g = (byte)(255.0f * v.y());
			b = (byte)(255.0f * v.z());
			a = (byte)(255.0f * v.w());
		}

		uint32 get() const { return data; }
		operator uint32() const { return get(); }
		operator Vector() const { return Vector((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f); }

		byte& R() { return r; }
		byte& G() { return g; }
		byte& B() { return b; }
		byte& A() { return a; }

		byte R() const { return r; }
		byte G() const { return g; }
		byte B() const { return b; }
		byte A() const { return a; }

	};
}