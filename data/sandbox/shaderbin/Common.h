/*
	Common types/functions
*/

#ifdef COMMON_H
#else
#define COMMON_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Shader Constants
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SLOT_PER_SCENE		register(b0)
#define SLOT_PER_MESH		register(b1)
#define SLOT_PER_MATERIAL	register(b2)

/*
	Direct light structure
*/
struct DirectLight
{
	float4 colour;
	float4 dir;
};

/*
	Dynamic point light structure
*/
struct DynamicLight
{
	float4 colour;
	float4 pos;

	int enabled;

	//Attenuation factors
	float attConstant;
	float attLinear;
	float attQuadratic;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SceneParams
{
	matrix view;
	matrix projection;
	float4 ambient;
	DirectLight directLight;
	DynamicLight dynamicLights[4];
};

struct MeshParams
{
	matrix world;
};

struct MaterialParams
{
	
};

cbuffer Scene : SLOT_PER_SCENE
{
	SceneParams scene;
};

cbuffer Mesh : SLOT_PER_MESH
{
	MeshParams mesh;
};

cbuffer Material : SLOT_PER_MATERIAL
{
	//MaterialParams material;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float3 phongShading(float3 normal, float3 vdir, float3 ldir, float3 lcolour)
{
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
	return phongShading(
		normal,
		normalize(vpos),
		normalize(source.pos.xyz - vpos),
		source.colour.rgb
	) * computeAttenuation(
		distance(source.pos.xyz, vpos),
		source
	);
}

float3 accumulateDynamicLights(float3 vnormal, float3 vpos)
{
	float3 accum = float3(0,0,0);

	for (int i = 0; i < 4; i++)
	{
		accum += (scene.dynamicLights[i].enabled != 0) ? computeDynamicLight(vnormal, vpos, scene.dynamicLights[i]) : float3(0,0,0);
	}

	return accum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif