/*
	Textured quad effect
*/

#include "effect_base.fxh"

cbuffer SceneParams : register(c0)
{
	CScene scene;
}

TextureCube testCube : register(t0);
SamplerState texSamplerWrap : register(s0);
SamplerState texSamplerClamp : register(s1);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	uint vertexID : SV_VertexID;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float3 view : VIEW;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;
	
	float2 texcoord = float2( (input.vertexID << 1) & 2, input.vertexID & 2 );
	output.pos = float4( texcoord * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );
	
	//output.view = mul(float3(1,1,1), (float3x3)scene.view);
	
	float3 unprojected = mul(output.pos, scene.invProjection).xyz;
	output.view = mul(unprojected, scene.invView);
	
	return output;
}

float4 PS(PSinput input) : SV_TARGET
{
	float4 colour = testCube.Sample(texSamplerClamp, input.view);
	return colour;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////