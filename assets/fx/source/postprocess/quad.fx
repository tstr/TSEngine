/*
	Textured quad effect
*/

#include "..\effect_base.fxh"

cbuffer SceneParams : register(b0)
{
	CScene scene;
}

#include "postfx_base.fxh"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Texture2D colourBuffer : register(t0);
Texture2D depthBuffer : register(t1);
TextureCube testCube : register(t2);

SamplerState texSampler : register(s0);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	uint vertexID : SV_VertexID;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;
	
	float2 texcoord = float2( (input.vertexID << 1) & 2, input.vertexID & 2 );
	output.pos = float4( texcoord * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );
	output.texcoord = texcoord;
	
	return output;
}

float4 PS(PSinput input) : SV_TARGET
{
	//float depth = sampleToDepth(depthBuffer.Sample(texSampler, input.texcoord).r) / 30;
	float4 colour = colourBuffer.Sample(texSampler, input.texcoord);
	//float depth = linearDepth(testCube.Sample(texSampler, float3(input.texcoord.xy, 1.0f)).r);
	
	/*
	float4 ColourFog = float4(0.2f, 0.2f, 0.2f, 1.0f);
	float density = 0.05;
	float fog = 1.0f / exp(depth * density);
	fog = clamp(fog, 0.0, 1.0);
	return lerp(ColourFog, colour, fog);
	*/
	
	//return testCube.Sample(texSampler, float3(input.texcoord.xy, 1.0f));

	//return EmbossShaderFunction3x3(colourBuffer, texSampler, input.texcoord);
	
	return colour;
	//return float4(depth, depth, depth, 1.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
