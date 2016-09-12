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

float ComputeAttenuation(float ca, float la, float qa, float d)
{
	return 1.0f / (ca + (la * d) + (qa * d * d));
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

float4 PS(PSinput input) : SV_TARGET
{
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
	float3 ambient = float3(0.31f, 0.31f, 0.31f);
	float3 diffuse = float3(1.0f, 1.0f, 1.0f) * diffuseIntensity * attenuation;

	if (diffuseIntensity > 0.0f)
	{
		//Reflection Vector
		float3 reflection = normalize(2 * diffuseIntensity * normal - ldir);
		
		//float specularPow = ((1 - material.useSpecularMap) * material.specularPower) + ((material.useSpecularMap) * texSpecular.Sample(texSampler, texcoord));
		float specularPow = 128;
		
		//Calculate specular factor, using Phong shading algorithm
		float specularIntensity = pow(saturate(dot(reflection, -vdir)), specularPow) * attenuation;
		
		specular = specularIntensity.xxx;
	}
	
	//return colour;
	return (colour * float4((ambient + diffuse + specular).rgb, 1.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////