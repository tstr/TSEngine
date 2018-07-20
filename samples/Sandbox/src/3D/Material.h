/*
	Material types
*/

#pragma once

#include <tsgraphics/Colour.h>
#include <tsgraphics/Image.h>

namespace ts
{
	/*
		Basic phong material description
	*/
	struct PhongMaterial
	{
		ImageView diffuseMap;
		ImageView normalMap;
		ImageView specularMap;
		ImageView displacementMap;

		RGBA diffuseColour;
		RGBA ambientColour;
		RGBA specularColour;
		RGBA emissiveColour;
		float specularPower = 0.0f;

		bool enableAlpha = false;
	};
}
