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
SamplerState texSamplerWrap : register(s0);
SamplerState texSamplerClamp : register(s1);

float4 BlurFunction3x3(Texture2D tex, SamplerState texSampler, float2 texcoords)
{
	float w = 1280;
	float h = 720;

	// TOP ROW
	float4 s11 = tex.Sample(texSampler, texcoords + float2(-1.0f / w, -1.0f / h));	// LEFT
	float4 s12 = tex.Sample(texSampler, texcoords + float2(0, -1.0f / h));			// MIDDLE
	float4 s13 = tex.Sample(texSampler, texcoords + float2(1.0f / w, -1.0f / h)); 	// RIGHT

	// MIDDLE ROW
	float4 s21 = tex.Sample(texSampler, texcoords + float2(-1.0f / w, 0));	// LEFT
	float4 col = tex.Sample(texSampler, texcoords);							// DEAD CENTER
	float4 s23 = tex.Sample(texSampler, texcoords + float2(-1.0f / w, 0));	// RIGHT

	// LAST ROW
	float4 s31 = tex.Sample(texSampler, texcoords + float2(-1.0f / w, 1.0f / h)); // LEFT
	float4 s32 = tex.Sample(texSampler, texcoords + float2(0, 1.0f / h));			// MIDDLE
	float4 s33 = tex.Sample(texSampler, texcoords + float2(1.0f / w, 1.0f / h));  // RIGHT

	// Average the color with surrounding samples
	col = (col + s11 + s12 + s13 + s21 + s23 + s31 + s32 + s33) / 9;
	return col;
}

// Blurs using a 7x7 filter kernel
float4 BlurFunction7x7(Texture2D tex, SamplerState texSampler, float2 texcoords) : COLOR0
{
	float w = 1280;
	float h = 720;

	return (
		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     -3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     -3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     -3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(0,                   -3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      -3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      -3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      -3.0f / h)) +

		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     -2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     -2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     -2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(0,                   -2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      -2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      -2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      -2.0f / h)) +

		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     -1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     -1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     -1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(0,                   -1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      -1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      -1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      -1.0f / h)) +

		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     0)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     0)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     0)) +
		tex.Sample(texSampler, texcoords + float2(0,                   0)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      0)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      0)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      0)) +

		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(0,                   1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      1.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      1.0f / h)) +

		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(0,                   2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      2.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      2.0f / h)) +

		tex.Sample(texSampler, texcoords + float2(-3.0f / w,     3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-2.0f / w,     3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(-1.0f / w,     3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(0,                   3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(1.0f / w,      3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(2.0f / w,      3.0f / h)) +
		tex.Sample(texSampler, texcoords + float2(3.0f / w,      3.0f / h))
	) / 49;
}

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
	return BlurFunction3x3(colourBuffer, texSamplerClamp, input.texcoord);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
