/*
	Standard shader
	
	todo: conditional compilation for shaders
*/

#include "common.fxh"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cbuffer SceneParams : register(b0)
{
	SScene scene;
}

cbuffer MaterialParams : register(b1)
{
	SMaterial material;
}

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texSpecular : register(t2);
Texture2D texDisplacement : register(t3);

TextureCube shadowMap : register(t8);

SamplerState texSampler : register(s0);

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
	float2 texcoord : TEXCOORD;
	float4 posw : POSITION_VIEW;
	
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float3 normal : NORMAL;
};

struct PSoutput
{
	float4 colour : SV_TARGET0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ComputeAttenuation(float ca, float la, float qa, float d)
{
	return 1.0f / (ca + (la * d) + (qa * d * d));
}

float ComputeVSMShadowFactor(float2 moments, float fragDepth)
{
	//Surface is fully lit. as the current fragment is before the light occluder
	if (fragDepth <= moments.x)
		return 1.0f;
	
	//The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to calculate the probability of this pixel being lit
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance,0.0002);

	float d = fragDepth - moments.x; //t - mu
	float probability = variance / (variance + d*d);

	return probability;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;

	input.pos.w = 1;	//ensure translation is possible
	output.pos = mul(input.pos, scene.world);

	//Save world position of vertex
	output.posw = output.pos;
	output.pos = mul(output.pos, scene.view);
	output.pos = mul(output.pos, scene.projection);

	input.texcoord.y = 1.0f - input.texcoord.y;
	output.texcoord = input.texcoord;
	
	output.tangent = mul(input.tangent, (float3x3)scene.world);
	output.bitangent = mul(input.bitangent, (float3x3)scene.world);
	output.normal = mul(input.normal, (float3x3)scene.world);
	
	return output;
}

PSoutput PS(PSinput input)
{
	float2 texcoord = input.texcoord;
	float3 normal = input.normal;
	
	if (material.useDisplacementMap)
	{
		const float height_scale = 0.04f;
		
		float3x3 tbn = float3x3( normalize(input.tangent), normalize(input.bitangent), normalize(input.normal) ); //transforms tangent=>world space
		tbn = transpose(tbn); //world=>tangent space
		float3 viewDirectionTangent = normalize((input.posw - scene.eyePos).xyz);
		viewDirectionTangent = mul(viewDirectionTangent, tbn);
		float3 normalTangent = input.normal;
		normalTangent = mul(normalTangent, tbn);
		
		float2 dx = ddx(input.texcoord);
		float2 dy = ddy(input.texcoord);

		float minSamples = 5;
		float maxSamples = 50;
		
		//return float4((float)(miplevel) / 128, 0, 0, 1);
		//return float4(maxSamples / 64.0f, 0, 0, 1);
		
		float parallaxLimit = -length( viewDirectionTangent.xy ) / viewDirectionTangent.z;
		parallaxLimit *= height_scale;

		float2 maxOffsetDirection = normalize(viewDirectionTangent.xy) * parallaxLimit;
		
		int numSamples = (int)lerp(maxSamples, minSamples, dot( viewDirectionTangent, normalTangent));
		
		float stepSize = 1.0f / (float)numSamples;

		float rayHeightCurrent = 1.0f;
		float2 offsetCurrent = float2( 0, 0 );
		float2 offsetLast = float2( 0, 0 );

		float SampledHeightLast = 1;
		float SampledHeightCurrent = 1;

		int sampleCurrent = 0;
		
		while ( sampleCurrent < numSamples )
		{
			SampledHeightCurrent = texDisplacement.SampleGrad( texSampler, input.texcoord + offsetCurrent, dx, dy ).r;
			if ( SampledHeightCurrent > rayHeightCurrent )
			{
				float delta1 = SampledHeightCurrent - rayHeightCurrent;
				float delta2 = ( rayHeightCurrent + stepSize ) - SampledHeightLast;

				float ratio = delta1 / (delta1 + delta2);

				offsetCurrent = (ratio * offsetLast) + ((1.0f - ratio) * offsetCurrent);

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
	
	float4 L = scene.lightPos - input.posw;
	float2 depth = shadowMap.Sample(texSampler, -L.xyz).rg;
	
	float factor = ComputeVSMShadowFactor(depth, length(L.xyz));
	
	if (material.useNormalMap)
	{
		float3x3 tbn = float3x3( normalize(input.tangent), normalize(input.bitangent), normalize(input.normal) ); //transforms tangent=>world space
		
		float2 t = float2(texcoord.x, 1.0f - texcoord.y);
		
		float3 tangentNormal = texNormal.Sample(texSampler, t).xyz;
		tangentNormal = normalize(tangentNormal * 2 - 1); //convert 0~1 to -1~+1.
		
		//tbn = transpose(tbn); //inverse matrix - tangent to world
		normal = normalize(mul(tangentNormal ,tbn));
	}
	
	float4 colour = float4(texDiffuse.Sample(texSampler, texcoord).rgb, 1.0f);

	float3 ldir = normalize(scene.lightPos - input.posw);
	//float3 ldir = float3(0.1f, 0.4f, -0.4f);
	float3 vdir = normalize((input.posw - scene.eyePos).xyz);
	
	float distance = length(scene.lightPos - input.posw);
	float attenuation = ComputeAttenuation(scene.lightConstantAttenuation, scene.lightLinearAttenuation, scene.lightQuadraticAttenuation, distance);
	
	//Diffuse lighting
	float diffuseIntensity = dot(normalize(normal), ldir);
	float3 specular = float3(0, 0, 0);
	float3 ambient = scene.globalAmbientColour;
	float3 diffuse = scene.lightColour * diffuseIntensity * attenuation * factor;

	//Specular lighting
	float3 reflection = normalize(2 * diffuseIntensity * normal - ldir); //Reflection Vector
	
	//Specular power value lies between 1 and 8192
	float specularPower = ((1 - material.useSpecularMap) * material.specularPower) + ((material.useSpecularMap) * pow(2, 13.0f * min(texSpecular.Sample(texSampler, texcoord).r, 1.0f)));
	//float specularPower = 128;

	//Calculate specular factor, using Phong shading algorithm
	float specularIntensity = pow(saturate(dot(reflection, -vdir)), specularPower) * attenuation * factor;
	
	specular = specularIntensity.xxx;

	PSoutput output = (PSoutput)0;
	output.colour = (colour * float4((ambient + diffuse + specular).rgb, 1.0f));
	return output;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////