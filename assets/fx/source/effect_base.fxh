/*
	Scene utils
*/

#ifndef _FX_BASE_H_
#define _FX_BASE_H_

#define MATERIAL_TEX_DIFFUSE (1 << 0)
#define MATERIAL_TEX_NORMAL (1 << 1)
#define MATERIAL_TEX_SPECULAR (1 << 2)
#define MATERIAL_TEX_DISPLACE (1 << 3)

struct CScene
{
	matrix world;
	matrix invWorld;
	matrix view;
	matrix invView;
	matrix projection;
	matrix invProjection;

	float4 lightPos;
	float4 lightColour ;
	float4 globalAmbientColour;
	float4 eyePos;
	
	float lightConstantAttenuation;
	float lightLinearAttenuation;
	float lightQuadraticAttenuation;
	
	float resolutionW;
	float resolutionH;
};

struct CMaterial
{
	float4 diffuseColour;
	float4 ambientColour;
	float4 specularColour;
	float specularPower;
	uint flags;
};

struct CPostprocess
{
	uint resX;
	uint resY;
};

#define SHADOW_FAR_PLANE 100.0f;
#define SHADOW_NEAR_PLANE 0.1f;

float vectorToDepth(float3 dir)
{
	float3 AbsVec = abs(dir);
    float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

    const float f = SHADOW_FAR_PLANE;
    const float n = SHADOW_NEAR_PLANE;
    float NormZComp = (f+n) / (f-n) - (2*f*n)/(f-n)/LocalZcomp;
    return (NormZComp + 1.0) * 0.5;
}

/*
float sampleToDepth(float depthSample)
{
	const float zFar = SHADOW_FAR_PLANE;
	const float zNear = SHADOW_NEAR_PLANE;

    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

float depthToSample(float linearDepth)
{
	const float zFar = SHADOW_FAR_PLANE;
	const float zNear = SHADOW_NEAR_PLANE;
	
    float nonLinearDepth = (zFar + zNear - 2.0 * zNear * zFar / linearDepth) / (zFar - zNear);
    nonLinearDepth = (nonLinearDepth + 1.0) / 2.0;
    return nonLinearDepth;
}
*/

float4 GammaCorrection(float4 c)
{
	//return float4(pow( c.rgb, (1.0f / 2.2f) ), c.a);
	return c;
}

#endif