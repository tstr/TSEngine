/*
	Shader Lighting Functions
*/

#include "Common.h"

#ifdef LIGHTING_H
#else
#define LIGHTING_H

///////////////////////////////////////////////////////////////////////////////

struct Surface
{
	float4 pos;
	float3 normal;
	float4 colour;
};

/*
	Compute lighting contribution at a given point using the Phong Reflection Algorithm
*/
float3 phongShading(float3 normal, float3 vdir, float3 ldir, float3 lcolour)
{	/*
	//Calculate diffuse lighting
	float diffuseIntensity = saturate(dot(normalize(input.vnorm), ldir));
	float specularIntensity = 0.0f;
	//return float4(diffuseIntensity.xxx, 1.0f);

	//Calculate specular lighting
	if (diffuseIntensity > 0.0f)
	{
	float3 reflection = normalize(2 * diffuseIntensity * input.vnorm - ldir); //Reflection Vector

	const float specularPower = 128;

	//Calculate specular factor, using Phong shading algorithm
	specularIntensity = pow(saturate(dot(reflection, -vdir)), specularPower);
	}

	return (scene.ambient * baseColour) + (baseColour * scene.direct.colour * (diffuseIntensity + specularIntensity));
	*/
	//return float4(input.vnorm, 1.0f);

	//Calculate diffuse lighting
	float diffuseTerm = saturate(dot(normal, ldir));
	float specularTerm = 0.0f;
	//return float4(diffuseIntensity.xxx, 1.0f);

	//Calculate specular lighting
	if (diffuseTerm > 0.0f)
	{
		float3 reflection = normalize(2 * diffuseTerm * normal - ldir); //Reflection Vector

		const float specularPower = 64;

		//Calculate specular factor, using Phong shading algorithm
		specularTerm = pow(saturate(dot(reflection, -vdir)), specularPower);
	}

	return lcolour * (diffuseTerm + specularTerm).xxx;
}


float3 computeDirectLight(float3 vnormal, float3 vpos, DirectLight source)
{
	return phongShading(
		vnormal,
		normalize(vpos),
		normalize(-source.dir),
		source.colour
	);
}

float computeAttenuation(float dist, DynamicLight source)
{
	return 1.0f / (source.attConstant + (source.attLinear * dist) + (source.attQuadratic * dist * dist));
}

float3 computeDynamicLight(float3 normal, float3 vpos, DynamicLight source)
{
	float3 illum = phongShading(
		normal,
		normalize(vpos),
		normalize(source.pos.xyz - vpos),
		source.colour.rgb
	);

	float attenuation = computeAttenuation(
		distance(source.pos.xyz, vpos),
		source
	);

	return illum * attenuation;
}

float3 accumulateDynamicLights(float3 vnormal, float3 vpos)
{
	float3 accum = float3(0, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		accum += (scene.dynamicLights[i].enabled != 0) ? computeDynamicLight(vnormal, vpos, scene.dynamicLights[i]) : float3(0, 0, 0);
	}

	return accum;
}

///////////////////////////////////////////////////////////////////////////////

float4 computeLighting(Surface surface)
{
	float3 light = float3(0, 0, 0);

	light += computeDirectLight(surface.normal, surface.pos.xyz, scene.directLight);
	light += accumulateDynamicLights(surface.normal, surface.pos.xyz);
	//light = computeDynamicLight(input.vnorm, input.vpos.xyz, scene.dynamicLights[0]);

	return float4(surface.colour.rgb * (scene.ambient.rgb + light), surface.colour.a);
}

///////////////////////////////////////////////////////////////////////////////

#endif
