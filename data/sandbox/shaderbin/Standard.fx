/*
	Standard Shader
*/

#include "Lighting.h"
#include "Shadows.h"
#include "CommonLayouts.h"

Texture2D diffuseMap : register(t0);

SamplerState texsample : register(s0);

/*
	Vertex Stage
*/
[stage("vertex")]
PixelInput_PosTexNorm VS(VertexInput_PosTexNorm input)
{
	PixelInput_PosTexNorm output = (PixelInput_PosTexNorm)0;
	
	//ensure translation is possible
	input.pos.w = 1;
	output.pos = input.pos;
	
	//transform position
	output.pos = mul(output.pos, mesh.world);
	output.pos = mul(output.pos, scene.view);

	//transform normal
	output.vnorm = input.normal;
	output.vnorm = mul(output.vnorm, (float3x3)mesh.world);
	output.vnorm = mul(output.vnorm, (float3x3)scene.view);
	output.vnorm = normalize(output.vnorm);

	//save view position of vertex
	output.vpos = output.pos;
	output.pos = mul(output.pos, scene.projection);
	output.lpos = mul(output.vpos, scene.directLightView);
	
	output.texcoord = input.texcoord;
	output.texcoord.y = 1.0f - output.texcoord.y;

	return output;
}

/*
	Pixel Stage
*/
[stage("pixel")]
float4 PS(PixelInput_PosTexNorm input) : SV_Target0
{
	float factor = calculateDirectLightShadow(input.lpos);

	Surface s;
	s.pos = input.vpos;
	s.normal = normalize(input.vnorm);
	s.colour = diffuseMap.Sample(texsample, input.texcoord);
	return computeLighting(s, factor);
}

