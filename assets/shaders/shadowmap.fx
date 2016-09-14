/*
	Shadow map effect
*/

#include "common.fxh"

cbuffer SceneParams : register(b0)
{
	SScene scene;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	float4 pos : POSITION;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float4 posview : POSITION_VIEW;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;
	
	input.pos.w = 1;	//ensure translation is possible
	
	output.pos = mul(input.pos, scene.world);
	output.pos = mul(output.pos, scene.view);
	
	output.posview = output.pos;
	//output.posview.w = 0;
	
	output.pos = mul(output.pos, scene.projection);
	
	return output;
}

float2 PS(PSinput input) : SV_TARGET
{
	//float factor = saturate(dot(normalize(scene.lightPos - input.pos), normalize(input.pos - scene.eyePos)));
	//float depth = input.pos.z / input.pos.w;
	float dist = length(input.posview);
	
	float m1 = dist;
	float m2 = dist * dist;
	
	float dx = ddx(dist);
	float dy = ddy(dist);
	m2 += ((dx * dx + dy * dy) / 4.0f);
	
	return float2(m1, m2);
}