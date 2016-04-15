/*
	Text rendering shader
*/

#include "effect_base.fxh"

Texture2D atlas : register(t0);
SamplerState texsampler : register(s0);

cbuffer SceneParams : register(c0)
{
	CScene scene;
}

struct VSinput
{
	float4 pos : POSITION;
	float4 colour : COLOUR;
	float2 texcoord : TEXCOORD;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float4 colour : COLOUR;
	float2 texcoord : TEXCOORD;
};


PSinput VS(VSinput input, uint vertexID : SV_VertexID)
{
	PSinput output;
	
	input.pos.w = 1;
	output.pos = input.pos;
	output.pos = mul(output.pos, scene.world);
	output.pos = mul(output.pos, scene.view);
	output.pos = mul(output.pos, scene.projection);
	
	output.colour = input.colour;
	output.texcoord = input.texcoord;
	return output;
}

float contour(in float d, in float w)
{
	// smoothstep(lower edge0, upper edge1, x)
	return smoothstep(0.5 - w, 0.5 + w, d);
}

float samp(in float2 uv, float w)
{
    return contour(atlas.Sample(texsampler, uv).r, w);
}

float4 PS(PSinput input) : SV_Target0
{
	float gcenter = 0.5f;
	
    float dist = atlas.Sample(texsampler, input.texcoord).r;
	float smoothing = fwidth(dist);
	//float smoothing = 0.7 * length(float2(ddx(dist), ddy(dist)));
	//float alpha = smoothstep(gcenter - smoothing, gcenter + smoothing, dist);
	
	//*
	float width = fwidth(dist);
	float alpha = contour( dist, width );
	
    // Supersample, 4 extra points
    float dscale = 0.354; // half of 1/sqrt2; you can play with this
    float2 duv = dscale * float2(ddx(input.texcoord) + ddy(input.texcoord));
    float4 box = float4(input.texcoord - duv, input.texcoord + duv);
    float asum = samp( box.xy, width ) + samp( box.zw, width ) + samp( box.xw, width ) + samp( box.zy, width );
    // weighted average, with 4 extra points having 0.5 weight each,
    alpha = (alpha + 0.5 * asum) / 3.0;
	//*/
	
	return float4(input.colour.rgb, alpha * input.colour.a);
	//return (input.colour * alpha);
	//return (input.colour * atlas.Sample(texsampler, input.texcoord).rrrr);
}