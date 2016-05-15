/*
	Lighting effect
*/

#include "effect_base.fxh"

#ifndef _FX_LIGHTS_
#define _FX_LIGHTS_

float ComputeVSMShadowFactor(float2 moments, float fragDepth)
{
	//Surface is fully lit. as the current fragment is before the light occluder
	if (fragDepth <= moments.x)
		return 1.0f;
	
	//The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to calculate the probability of this pixel being lit
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance,0.0002);

	float d = fragDepth - moments.x; //t - mu
	float probability = variance / (variance + d*d);

	return probability;


/*
	float2 lit = (float2)0.0f;
	float E_x2 = moments.y;				//E(x^2)
	float Ex_2 = moments.x * moments.x; //E(x)^2
	float variance = E_x2 - Ex_2;
	float mD = moments.x - fragDepth;
	float mD_2 = mD * mD;
	float p = variance / (variance + mD_2);
	lit.x = max( p, fragDepth <= moments.x );
	
	
	return lit.x;
//*/
}

float ComputeAttenuation(float ca, float la, float qa, float d)
{
	return 1.0f / (ca + (la * d) + (qa * d * d));
}

float4 ComputeAmbientLighting(float4 colour)
{
	return float4(scene.globalAmbientColour.rgb * colour.rgb, colour.a);
}

float FresnelSchlick(float f0, float fd90, float view)
{
	return f0 + (fd90 - f0) * pow(max(1.0f - view, 0.1f), 5.0f);
}

float4 ComputeLighting(float4 colour, float3 normal, float3 lightDirection, float3 viewDirection, float distance, float specularPow, float shadowFactor)
{
	/*
	float4 specular = float4(0, 0, 0, 0);
	float4 ambient = material.ambientColour + scene.globalAmbientColour;
	float diffuse = saturate(dot(normal, lightDirection));

	if (diffuse > 0.0f)
	{
		float4 reflection = normalize(reflect(-lightDirection, normal));
		float RdotV = max(0, dot(reflection, viewDirection));
		//specular = saturate(pow(RdotV, specularPower));

		//specular = specular * specularColour;
		//diffuse = diffuse * diffuseColour;
	}

	//Compute final lighting
	return ((ambientColour + specular + diffuse) * lightColour * colour);
	*/

	float3 diffuse = float3(0,0,0);
	float3 specular = float3(0,0,0);
	float3 ambient = float3(0,0,0);
	
	float4 finalColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	//Intensity
	float diffuseIntensity = 0;
	float specularIntensity = 0;
	
	float attenuation = 1.0f;

	if (distance > 0.0f)
	{
		attenuation = ComputeAttenuation(scene.lightConstantAttenuation, scene.lightLinearAttenuation, scene.lightQuadraticAttenuation, distance);
	}
	
	//ambient = material.ambientColour * scene.globalAmbientColour;
	ambient = scene.globalAmbientColour.rgb;

	diffuseIntensity = saturate(dot(normal, lightDirection)) * attenuation;
	
	if (diffuseIntensity > 0.0f)
	{
		//Initial diffuse component
		diffuse = diffuseIntensity * material.diffuseColour.rgb * scene.lightColour.rgb;
		
		//*
		//Reflection Vector
		float3 reflection = normalize(2 * diffuseIntensity * normal - lightDirection);
		
		//Calculate specular factor, using Phong shading algorithm
		specularIntensity = pow(saturate(dot(reflection, -viewDirection)), specularPow) * attenuation;
		
		specular = specularIntensity.xxx;
		//*/
		
		//if (specularPow > 1.0f)
		//	return float4(diffuseIntensity, 0, 0, 1);
		//else
		//	return float4(0, diffuseIntensity, 0, 1);
		
		/*
		float3 h = normalize(lightDirection + viewDirection);
		float NdotH = saturate(dot(normal, h));
		
		float roughness = 0.65f;

		float rough2 = max(roughness * roughness, 2.0e-3f); // capped so spec highlights don't disappear
		float rough4 = rough2 * rough2;

		float d = (NdotH * rough4 - NdotH) * NdotH + 1.0f;
		float D = rough4 / (PI * (d * d));

		// Fresnel
		float3 reflectivity = specularPow;
		float fresnel = 1.0;
		float NdotL = saturate(dot(normal, lightDirection));
		float LdotH = saturate(dot(lightDirection, h));
		float NdotV = saturate(dot(normal, viewDirection));
		float3 F = reflectivity + (fresnel - fresnel * reflectivity) * exp2((-5.55473f * LdotH - 6.98316f) * LdotH);

		// geometric / visibility
		float k = rough2 * 0.5f;
		float G_SmithL = NdotL * (1.0f - k) + k;
		float G_SmithV = NdotV * (1.0f - k) + k;
		float G = 0.25f / (G_SmithL * G_SmithV);
		
		
		float energyBias = lerp(0.0f, 0.5f, roughness);
		float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
		float fd90 = energyBias + 2.0f * (LdotH * LdotH) * roughness;
		float f0 = 1.0f;

		float lightScatter = FresnelSchlick(f0, fd90, NdotL);
		float viewScatter = FresnelSchlick(f0, fd90, NdotV);
		
		diffuse = lightScatter * viewScatter * energyFactor;
		specular = G * D * F * attenuation;
		//*/
	}
	
	//finalColour = finalColour * colour * scene.lightColour;
	finalColour = float4((ambient + (diffuse + specular) * shadowFactor), 1.0f) * colour;

	//colour = (colour * diffuseIntensity) + specularIntensity;

	return finalColour;
}

float2 ComputeParallaxTexcoords()
{
	
}

#endif