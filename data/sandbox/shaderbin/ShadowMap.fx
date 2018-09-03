/*
	Variance Shadow Map writer
*/

#include "Common.h"
#include "Shadows.h"

///////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	float4 pos : POSITION;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float4 vpos : VIEW_POSITION;
};

///////////////////////////////////////////////////////////////////////////////

[stage("VERTEX")]
PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;

	input.pos.w = 1;	//ensure translation is possible
	
	//Calculate position in view space
	output.vpos = mul(input.pos, mesh.world);
	output.vpos = mul(output.vpos, scene.view);
	//Calculate position in clip space
	output.pos = mul(output.vpos, scene.projection);

	//output.vpos.w = 0;

	return output;
}

[stage("PIXEL")]
float2 PS(PSinput input) : SV_TARGET
{
	//float factor = saturate(dot(normalize(scene.lightPos - input.pos), normalize(input.pos - scene.eyePos)));
	//float depth = length(input.vpos.xyz) / SHADOW_FAR_PLANE;

	float d = input.pos.z / input.pos.w;

	//*
	float m1 = d;
	float m2 = d * d;

	float dx = ddx(d);
	float dy = ddy(d);
	m2 += ((dx * dx + dy * dy) / 4.0f);

	return float2(m1, m2);
	/*/
	return float2(d, d * d);
	//*/
}

///////////////////////////////////////////////////////////////////////////////