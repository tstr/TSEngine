/*
	Shadow mapping functions
*/

#ifdef SHADOWS_H
#else
#define SHADOWS_H

///////////////////////////////////////////////////////////////////////////////

#define SHADOW_SLOT register(t7)

#define SHADOW_FAR_PLANE 150.0f;
#define SHADOW_NEAR_PLANE 0.1f;

/*
float vectorToDepth(float3 dir)
{
	float3 AbsVec = abs(dir);
	float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

	const float f = SHADOW_FAR_PLANE;
	const float n = SHADOW_NEAR_PLANE;
	float NormZComp = (f + n) / (f - n) - (2 * f*n) / (f - n) / LocalZcomp;
	return (NormZComp + 1.0) * 0.5;
}
*/

float computeVSMShadowFactor(float2 moments, float fragDepth)
{
	//Surface is fully lit. as the current fragment is before the light occluder
	if (fragDepth <= moments.x)
		return 1.0f;

	//The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to calculate the probability of this pixel being lit
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, 0.0002);

	float d = fragDepth - moments.x; //t - mu
	float probability = variance / (variance + d * d);

	return probability;
}

///////////////////////////////////////////////////////////////////////////////
//*

Texture2D shadowMap : SHADOW_SLOT;
SamplerState shadowSampler : register(s1);

float calculateDirectLightShadow(float4 lightSpacePos)
{
	const float bias = 0.000001f;

	float factor = 0.0;

	//position in light view space
	float4 lvpos = lightSpacePos;
	float3 lvdir = lvpos / SHADOW_FAR_PLANE;
	float lvdepth = length(lvdir);

	if (lvdir.z > 0)
	{
		float2 tex;
		tex.x = (lvpos.x / lvpos.w) / 2.0f + 0.5f;
		tex.y = (-lvpos.y / lvpos.w) / 2.0f + 0.5f;

		//If in bounds
		if ((saturate(tex.x) == tex.x) && (saturate(tex.y) == tex.y))
		{
			//Light view position depth
			//float ld = (lvpos.z / lvpos.w) / SHADOW_FAR_PLANE;
			float ld = length(lvdir);
			//Sampled depth
			float2 sd = shadowMap.Sample(shadowSampler, tex).r;

			//factor = computeVSMShadowFactor(sd, ld);
			factor = (sd.r < ld) ? factor : 1;
		}
	}

	return factor;
}
//*/

///////////////////////////////////////////////////////////////////////////////

#endif