/*
	Standard Normal Mapped Shader
*/

#include "Lighting.h"
#include "Shadows.h"
#include "CommonLayouts.h"

Texture2D diffuseMap : register(t0);
Texture2D normalMap  : register(t1);

SamplerState texsample : register(s0);

/*
	Vertex Stage
*/
[stage("vertex")]
PixelInput_PosTexNormTangent VS(VertexInput_PosTexNormTangent input)
{
	PixelInput_PosTexNormTangent output = (PixelInput_PosTexNormTangent)0;

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
	//transform tangent
	output.vtangent = input.tangent;
	output.vtangent = mul(output.vtangent, (float3x3)mesh.world);
	output.vtangent = mul(output.vtangent, (float3x3)scene.view);
	output.vtangent = normalize(output.vtangent);
	//calculate bitangent vector
	output.vbitangent = normalize(cross(output.vnorm, output.vtangent));

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
float4 PS(PixelInput_PosTexNormTangent input) : SV_Target0
{
	float factor = calculateDirectLightShadow(input.lpos);

	//transforms tangent space => view space
	float3x3 tbn = float3x3(normalize(input.vtangent), normalize(input.vbitangent), normalize(input.vnorm)); 
	//tbn = transpose(tbn); //inverse matrix - view to tangent

	//Flip v coord - todo: fix issue with inverted normal maps
	input.texcoord.y = 1.0f - input.texcoord.y;

	float3 tangentNormal = normalMap.Sample(texsample, input.texcoord).xyz;
	tangentNormal = normalize(tangentNormal * 2 - 1); //convert 0~1 to -1~+1.
	
	input.texcoord.y = 1.0f - input.texcoord.y; //restore

	Surface s;
	s.pos = input.vpos;
	s.normal = normalize(mul(tangentNormal, tbn)); //convert tangent space normal to view space
	s.colour = diffuseMap.Sample(texsample, input.texcoord);

	return computeLighting(s, factor);

