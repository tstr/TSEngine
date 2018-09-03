/*
	Shadow mapping functions
*/

#ifdef SHADOWS_H
#else
#define SHADOWS_H

///////////////////////////////////////////////////////////////////////////////

#define SHADOW_SLOT register(t7)

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

float calculateDirectLightShadow(float4 lvpos)
{
	const float bias = 0.000001f;

	float factor = 0.0;

	//float lvdepth = length(lvdir) / SHADOW_FAR_PLANE;

	//position in light-view space
	float lvd = lvpos.z / lvpos.w;

	float2 tex;
	tex.x = (lvpos.x / lvpos.w) / 2.0f + 0.5f;
	tex.y = (-lvpos.y / lvpos.w) / 2.0f + 0.5f;

	//If in bounds
	if ((saturate(tex.x) == tex.x) && (saturate(tex.y) == tex.y))
	{
		//Sampled depth
		float2 sd = shadowMap.Sample(shadowSampler, tex).r;
		
		//factor = computeVSMShadowFactor(sd, lvdepth);
		factor = (lvd <= (sd.r + bias)) ? 1 : factor;
	}

	return factor;
}
//*/

///////////////////////////////////////////////////////////////////////////////

#endif