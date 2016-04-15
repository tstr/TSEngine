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
Texture2D texNormal : register(t1);
Texture2D texDisplacement : register(t2);
Texture2D texSpecular : register(t3);

SamplerState texSampler : register(s0);

TextureCube shadowMap : register(t7);
TextureCube skyBox : register(t8);

#include "effect_lights.fxh"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VSinput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 posw : POSITION_WORLD;
	
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float3 normal : NORMAL;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;

	input.pos.w = 1;	//ensure translation is possible
	output.pos = mul(input.pos, scene.world);

	//Save world position of vertex and vertex normal
	output.posw = output.pos;
	output.pos = mul(output.pos, scene.view);
	output.pos = mul(output.pos, scene.projection);

	input.texcoord.y = 1.0f - input.texcoord.y;
	output.texcoord = input.texcoord;
	
	output.tangent = mul(input.tangent, (float3x3)scene.world);
	output.bitangent = mul(input.bitangent, (float3x3)scene.world);
	output.normal = mul(input.normal, (float3x3)scene.world);

	return output;
}

float4 PS(PSinput input) : SV_TARGET
{
	//bool visible = false;
	
	// Set the bias value for fixing the floating point precision issues.
	const float bias = 1.0f / 2048; //0.001f;
	
	float4 L = scene.lightPos - input.posw;
	float2 depth = shadowMap.Sample(texSampler, -L.xyz).rg;

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
	
	//Height
	//float height = texDisplacement.Sample(texSampler, input.texcoord).r;
	
	float3x3 tbn = float3x3( normalize(input.tangent), normalize(input.bitangent), normalize(input.normal) ); //transforms tangent=>world space
	tbn = transpose(tbn); //inverse matrix - world to tangent
	
	float3 viewDirection = normalize((input.posw - scene.eyePos).xyz);
	float3 viewDirectionTangent = normalize(mul(viewDirection, tbn));    //tangent space view direction vector
	float3 normalTangent = normalize(mul(normalize(input.normal), tbn)); //tangent space vertex normal
	
	float2 texcoord = input.texcoord;
	
	[branch]
	if (material.flags & MATERIAL_TEX_DISPLACE)
	{
		const float height_scale = 0.04f;
		const float height_offset = 0.01f;
	
		float2 dx = ddx(input.texcoord);
		float2 dy = ddy(input.texcoord);
		
		float minSamples = 10;
		float maxSamples = 50;
		
		uint width, height; 
		texDisplacement.GetDimensions(width, height);
		float miplevel = log(max(1, dx * (float)width));
		
		float parallaxLimit = -length( viewDirectionTangent.xy ) / viewDirectionTangent.z;
		parallaxLimit *= height_scale;
		
		float2 offsetDirection = normalize(viewDirectionTangent.xy);
		float2 maxOffsetDirection = offsetDirection * parallaxLimit;
		
		int numSamples = (int)lerp(maxSamples, minSamples, dot( viewDirectionTangent, normalTangent ));
		
		float stepSize = 1.0f / (float)numSamples;

		float rayHeightCurrent = 1.0f;
		float2 offsetCurrent = float2( 0, 0 );
		float2 offsetLast = float2( 0, 0 );

		float SampledHeightLast = 1;
		float SampledHeightCurrent = 1;

		int sampleCurrent = 0;
		
		while ( sampleCurrent < numSamples )
		{
			SampledHeightCurrent = texDisplacement.SampleGrad( texSampler, input.texcoord + offsetCurrent, dx, dy ).r;
			if ( SampledHeightCurrent > rayHeightCurrent )
			{
				float delta1 = SampledHeightCurrent - rayHeightCurrent;
				float delta2 = ( rayHeightCurrent + stepSize ) - SampledHeightLast;

				float ratio = delta1 / (delta1 + delta2);

				offsetCurrent = (ratio) * offsetLast + (1.0f - ratio) * offsetCurrent;

				sampleCurrent = numSamples + 1;
				//break;
			}
			else
			{
				sampleCurrent++;

				rayHeightCurrent -= stepSize;

				offsetLast = offsetCurrent;
				offsetCurrent += stepSize * maxOffsetDirection;

				SampledHeightLast = SampledHeightCurrent;
			}
		}
		
		texcoord = input.texcoord + offsetCurrent;
	}
	
	float3 normal = normalize(input.normal);

	float4 colour = (material.flags & MATERIAL_TEX_DIFFUSE) ? texColour.Sample(texSampler, texcoord) : material.diffuseColour;
	float specularPower = (material.flags & MATERIAL_TEX_SPECULAR) ? (material.specularPower * texSpecular.Sample(texSampler, texcoord).r) : material.specularPower;
	//float specularPower = (material.flags & MATERIAL_TEX_SPECULAR) ? (sqrt(texSpecular.Sample(texSampler, texcoord).r) * 8192) : material.specularPower;
	
	//specularPower = material.specularPower;
	
	if (material.flags & MATERIAL_TEX_NORMAL)
	{
		//float3x3 tbn = float3x3( normalize(input.tangent), normalize(input.bitangent), normalize(input.normal) ); //transforms tangent=>world space
		
		float2 t = float2(texcoord.x, 1.0f - texcoord.y);
		
		float3 tangentNormal = texNormal.Sample(texSampler, t).xyz;
		tangentNormal = normalize(tangentNormal * 2 - 1); //convert 0~1 to -1~+1.
		
		tbn = transpose(tbn); //inverse matrix - tangent to world
		normal = normalize(mul(tangentNormal ,tbn));
	}
	
	//normal = input.normal;
	//specularPower = material.specularPower;
	
	//Check if pixel is visible
	//if ((length(L.xyz) - bias) < depth.r)
	//if (ComputeVSMShadowFactor(depth.x, vectorToDepth(L.xyz)))
	
	float factor = ComputeVSMShadowFactor(depth, length(L.xyz));
	
	float3 lightdir = (scene.lightPos - input.posw).xyz;
	float distance = length(lightdir);
	
	colour = ComputeLighting(
		colour,
		normal,
		lightdir / distance,//Light Direction - relative to pixel world position
		viewDirection,//normalize(scene.eyePos - input.posw), //View direction - from pixel to camera in world space
		distance,
		specularPower,
		factor
	);
	
	/*
	//reflect ray around normal from eye to surface
	float3 incident_eye = normalize(input.posw.xyz);
	float3 normal = normalize(input.normal.xyz);
	float3 reflected = reflect (incident_eye, normal);
	//convert from eye to world space
	reflected = mul(reflected, (float3x3)scene.invView);
	colour = colour + skyBox.Sample(texSampler, reflected);
	*/
	
	//Alpha test - if alpha value of pixel colour is less than 0.9 then discard pixel
	clip((colour.a < 0.9f) ? -1 : 1);
	
	return colour;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////