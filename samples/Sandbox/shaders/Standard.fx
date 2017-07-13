/*
	Standard Shader
*/

#include "CommonLayouts.h"
#include "Common.h"

Texture2D diffuse : register(t0);
SamplerState texsample : register(s0);

/*
	Vertex Stage
*/
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
	
	output.texcoord = input.texcoord;
	output.texcoord.y = 1.0f - output.texcoord.y;

	return output;
}

/*
	Pixel Stage
*/
float4 PS(PixelInput_PosTexNorm input) : SV_Target0
{
	//Normalize view space normal
	input.vnorm = normalize(input.vnorm);

	float4 baseColour = diffuse.Sample(texsample, input.texcoord);
	
	float3 light = float3(0, 0, 0);
	
	light += computeDirectLight(input.vnorm, input.vpos.xyz, scene.directLight);
	light += accumulateDynamicLights(input.vnorm, input.vpos.xyz);
	//light = computeDynamicLight(input.vnorm, input.vpos.xyz, scene.dynamicLights[0]);

	return float4(baseColour.rgb * (scene.ambient.rgb + light), baseColour.a);

	/*
	//Calculate diffuse lighting
	float diffuseIntensity = saturate(dot(normalize(input.vnorm), ldir));
	float specularIntensity = 0.0f;
	//return float4(diffuseIntensity.xxx, 1.0f);

	//Calculate specular lighting
	if (diffuseIntensity > 0.0f)
	{
		float3 reflection = normalize(2 * diffuseIntensity * input.vnorm - ldir); //Reflection Vector

		const float specularPower = 128;

		//Calculate specular factor, using Phong shading algorithm
		specularIntensity = pow(saturate(dot(reflection, -vdir)), specularPower);
	}

	return (scene.ambient * baseColour) + (baseColour * scene.direct.colour * (diffuseIntensity + specularIntensity));
	*/
	//return float4(input.vnorm, 1.0f);
}

