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
	float4 diffuseColour;
	float4 ambientColour;
	float4 specularColour;
	float4 emissiveColour;
	float specularPower;
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
	MaterialParams material;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif