/*
	Shader
*/

#include "effect_base.fxh"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cbuffer SceneParams : register(b0)
{
	CScene scene;
}

cbuffer MaterialParams : register(b1)
{
	CMaterial material;
}

Texture2D texColour : register(t0);
SamplerState texSampler : register(s0);

TextureCube shadowMap : register(t7);

#include "effect_lights.fxh"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	float4 pos : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 posw : POSITION_WORLD;
	float4 normal : NORMAL_WORLD;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;

	input.pos.w = 1;	//ensure translation is possible
	input.normal.w = 0; //disallow translation of normal vector

	output.pos = mul(input.pos, scene.world);

	//Save world position of vertex and vertex normal
	output.posw = output.pos;
	output.normal = normalize(mul(input.normal, scene.world));

	output.pos = mul(output.pos, scene.view);
	output.pos = mul(output.pos, scene.projection);

	output.texcoord = input.texcoord;

	return output;
}

float4 PS(PSinput input) : SV_TARGET
{
	//bool visible = false;
	
	// Set the bias value for fixing the floating point precision issues.
	const float bias = 1.0f / 2048; //0.001f;
	
	float4 L = scene.lightPos - input.posw;
	float depth = shadowMap.Sample(texSampler, -L.xyz).x;

	/*
	bool visible = false;
	
	// Set the bias value for fixing the floating point precision issues.
    const float bias = 0.001f;
	
	float2 projectTexCoord;

    projectTexCoord.x =  input.lightPosView.x / input.lightPosView.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.lightPosView.y / input.lightPosView.w / 2.0f + 0.5f;
	
	//check if point is in bounds
	visible = (saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y);
	
	if (visible)
	{
		//Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
		//float depth = shadowMap.Sample(texSampler, projectTexCoord);
		
		float3 L = -(scene.lightPos - input.posw).xyz;
		float depth = shadowMap.Sample(texSampler, -L);
		
		//Calculate the depth of the light.
        float lightDepth = input.lightPosView.z / input.lightPosView.w;

        //Subtract the bias from the lightDepth.
        lightDepth = lightDepth - bias;
		
		visible = lightDepth < depth;
	}
	*/
	
	float4 colour;
	
	//Check if pixel is visible
	if ((vectorToDepth(L.xyz) - bias) < depth)
	{
		float3 L = (scene.lightPos - input.posw).xyz;
		float distance = length(L);
		
		colour = ComputeLighting(
			texColour.Sample(texSampler, input.texcoord),
			input.normal,
			L / distance,//Light Direction - relative to pixel world position
			normalize(scene.eyePos - input.posw), //View direction - from pixel to camera in world space
			length(L),
			material.specularPower
		);
	}
	else
	{
		colour = ComputeAmbientLighting(texColour.Sample(texSampler, input.texcoord));
	}

	return GammaCorrection(colour);
	//return float4(colour.rgb, (input.pos.z / input.pos.w));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////