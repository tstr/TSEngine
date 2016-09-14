/*
	Shader
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
	float4 L = scene.lightPos - input.posw;
	float2 depth = shadowMap.Sample(texSampler, -L.xyz).rg;
	
	float factor = ComputeVSMShadowFactor(depth, length(L.xyz));

	float2 texcoord = input.texcoord;
	float3 normal = input.normal;
	
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
	
	float diffuseIntensity = dot(normalize(normal), ldir);
	float3 specular = float3(0, 0, 0);
	float3 ambient = scene.globalAmbientColour;
	float3 diffuse = scene.lightColour * diffuseIntensity * attenuation * factor;

	if (diffuseIntensity > 0.0f)
	{
		//Reflection Vector
		float3 reflection = normalize(2 * diffuseIntensity * normal - ldir);
		
		//float specularPow = ((1 - material.useSpecularMap) * material.specularPower) + ((material.useSpecularMap) * texSpecular.Sample(texSampler, texcoord));
		float specularPow = 128;
		
		//Calculate specular factor, using Phong shading algorithm
		float specularIntensity = pow(saturate(dot(reflection, -vdir)), specularPow) * attenuation * factor;
		
		specular = specularIntensity.xxx;
	}
	
	PSoutput output = (PSoutput)0;
	output.colour = (colour * float4((ambient + diffuse + specular).rgb, 1.0f));
	return output;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////