/*
	Simple bump-map effect
*/

#include "effect_base.fxh"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cbuffer SceneParams : register(b0)
{
	CScene scene;
}

cbuffer MaterialParams : register(b1)
{
	CMaterial material;
}

Texture2D texColour : register(t0);
Texture2D texNormal : register(t1);
Texture2D texDisplace : register(t2);
Texture2D texSpecular : register(t3);

TextureCube shadowMap : register(t7);

SamplerState texSampler : register(s0);

#include "effect_lights.fxh"

#define PARALLAX_OCCLUSION

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float4 posWorld : POSITION_WORLD;
	float2 texcoord : TEXCOORD;
	
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float3 normal : NORMAL;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;

	input.pos.w = 1;	//ensure translation is possible
	
	output.pos = mul(input.pos, scene.world);
	output.posWorld = output.pos;
	output.pos = mul(output.pos, scene.view);
	output.pos = mul(output.pos, scene.projection);

	output.tangent = mul(input.tangent, (float3x3)scene.world);
	output.bitangent = mul(input.bitangent, (float3x3)scene.world);
	output.normal = mul(input.normal, (float3x3)scene.world);
	
	output.texcoord = input.texcoord;

	return output;
}

float4 PS(PSinput input) : SV_TARGET
{
	float4 colour = float4(0,0,0,0);
	
	// Set the bias value for fixing the floating point precision issues.
	const float bias = 1.0f / 2048; //0.001f;
	
	float3 L = (scene.lightPos - input.posWorld).xyz;
	float depth = shadowMap.Sample(texSampler, -L).x;
	
	//Height
	float height = texDisplace.Sample(texSampler, input.texcoord).r;
	const float height_scale = 0.04f;
	const float height_offset = 0.01f;

	float2 texcoord = input.texcoord;
	
	/*
	// compute derivations of the world position
	float3 p_dx = ddx(input.posWorld);
	float3 p_dy = ddy(input.posWorld);
	// compute derivations of the texture coordinate
	float2 tc_dx = ddx(input.texcoord);
	float2 tc_dy = ddy(input.texcoord);
	// compute initial tangent and bi-tangent
	float3 t = normalize( tc_dy.y * p_dx - tc_dx.y * p_dy );
	float3 b = normalize( tc_dy.x * p_dx - tc_dx.x * p_dy ); // sign inversion
	// get new tangent from a given mesh normal
	float3 n = normalize(input.normal);
	float3 x = cross(n, t);
	t = cross(x, n);
	t = normalize(t);
	// get updated bi-tangent
	x = cross(b, n);
	b = cross(n, x);
	b = normalize(b);
	float3x3 tbn = float3x3(t, b, n);
	*/
	
	//*
	float3x3 tbn = float3x3( normalize(input.tangent), normalize(input.bitangent), normalize(input.normal) ); //transforms tangent=>world space
	tbn = transpose(tbn); //inverse matrix - world to tangent
	
	float3 viewDirection = normalize((input.posWorld - scene.eyePos).xyz);
	float3 viewDirectionTangent = normalize(mul(viewDirection, tbn));
	float3 normalTangent = normalize(mul(normalize(input.normal), tbn));
	//*/
	
	#ifndef PARALLAX_OCCLUSION
	//Compute parallax
	texcoord += viewDirection.xy * ((height * height_scale) - height_offset);
	#else
	
	//Compute parallax occlusion
	{
		float minSamples = 8;
		float maxSamples = 40;
		
		float parallaxLimit = -length( viewDirectionTangent.xy ) / viewDirectionTangent.z;
		parallaxLimit *= height_scale;
		
		float2 offsetDirection = normalize(viewDirectionTangent.xy);
		float2 maxOffsetDirection = offsetDirection * parallaxLimit;
		
		int numSamples = (int)lerp(maxSamples, minSamples, dot( viewDirectionTangent, normalTangent ));
		
		float stepSize = 1.0f / (float)numSamples;
		
		float2 dx = ddx(input.texcoord);
		float2 dy = ddy(input.texcoord);
		
		float rayHeightCurrent = 1.0f;
		float2 offsetCurrent = float2( 0, 0 );
		float2 offsetLast = float2( 0, 0 );

		float SampledHeightLast = 1;
		float SampledHeightCurrent = 1;

		int sampleCurrent = 0;
		
		while ( sampleCurrent < numSamples )
		{
			SampledHeightCurrent = texDisplace.SampleGrad( texSampler, input.texcoord + offsetCurrent, dx, dy ).r;
			if ( SampledHeightCurrent > rayHeightCurrent )
			{
				float delta1 = SampledHeightCurrent - rayHeightCurrent;
				float delta2 = ( rayHeightCurrent + stepSize ) - SampledHeightLast;

				float ratio = delta1/(delta1+delta2);

				offsetCurrent = (ratio) * offsetLast + (1.0-ratio) * offsetCurrent;

				sampleCurrent = numSamples + 1;
				//break;
			}
			else
			{
				sampleCurrent++;

				rayHeightCurrent -= stepSize;

				offsetLast = offsetCurrent;
				offsetCurrent += stepSize * maxOffsetDirection;

				SampledHeightLast = SampledHeightCurrent;
			}
		}
		
		texcoord = input.texcoord + offsetCurrent;
	}
	#endif
	
	if((vectorToDepth(L) - bias) < depth)
	{
		float3 tangentNormal = texNormal.Sample(texSampler, texcoord).xyz;
		tangentNormal = normalize(tangentNormal * 2 - 1); //convert 0~1 to -1~+1.
		
		//float specularPow = material.specularPower;
		float specularPow = 128 * (1.0f - texSpecular.Sample(texSampler, texcoord));
		//float specularPow = (texSpecular.Sample(texSampler, texcoord));
		
		tbn = transpose(tbn); //tangent to world
		float3 normal = normalize(mul(tangentNormal ,tbn));
		
		float3 L = (scene.lightPos - input.posWorld).xyz;
		float distance = length(L);
		
		colour = ComputeLighting(
			texColour.Sample(texSampler, texcoord),
			normal,
			L / distance,//Light Direction - relative to pixel world position
			viewDirection,
			length(L),
			specularPow
		);
	}
	else
	{
		colour = ComputeAmbientLighting(texColour.Sample(texSampler, texcoord));
	}
	
	/*
	float4 ColourFog = float4(0.2, 0.2, 0.2, 1);
	float fogRange = length(input.posWorld);
	float fogDensity = 0.05;
	float fog = 1.0f / exp(fogRange * fogDensity);
	fog = clamp(fog, 0.0, 1.0);
	return lerp(ColourFog, colour, fog);
	*/

	return GammaCorrection(float4(colour.rgb, 1.0f));
}