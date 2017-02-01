//TextureCube tex : register(t0);
Texture2D tex : register(t0);
Texture2D texDisp : register(t1);
Texture2D texNorm : register(t2);
SamplerState texSampler : register(s0);

cbuffer uniforms : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float4 u_lightdir;
	float  u_tessScale;
	float  u_tessFactor;
	float  u_time;
}

struct VSinput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float3 tangent : TANGENT;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texcoord : TEXCOORD;
	float tessFactor : TESSFACTOR;
};

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;
	
	input.pos.w = 1;
	output.pos = mul(input.pos, world);
	output.normal = mul(input.normal, (float3x3)world);
	output.tangent = mul(input.tangent, (float3x3)world);
	output.texcoord = input.texcoord;
	
	//output.pos = mul(output.pos, view);
	//output.pos = mul(output.pos, projection);
	//output.normal = mul(output.normal, (float3x3)view);
	//output.tangent = mul(output.tangent, (float3x3)view);

	output.tessFactor = u_tessFactor;
	
	output.normal = normalize(output.normal);
	output.tangent = normalize(output.tangent);
	
	return output;
}

struct HSconstOutput 
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

HSconstOutput PatchHS( InputPatch<PSinput, 3> patch, uint PatchID : SV_PrimitiveID ) 
{
	HSconstOutput output; 
	output.edges[0] = 0.5f*(patch[1].tessFactor + patch[2].tessFactor);
	output.edges[1] = 0.5f*(patch[2].tessFactor + patch[0].tessFactor);
	output.edges[2] = 0.5f*(patch[0].tessFactor + patch[1].tessFactor);
	output.inside  = output.edges[0];
	/*
	output.edges[0] = u_tessFactor;
	output.edges[1] = u_tessFactor;
	output.edges[2]= u_tessFactor; 
	output.inside = u_tessFactor;
	*/
	return output; 
}

[domain("tri")] 
//[partitioning("integer")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]		//triangle topology with clockwise winding
[outputcontrolpoints(3)] 
[patchconstantfunc("PatchHS")]
PSinput HS( InputPatch<PSinput, 3> patch, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID ) 
{
	return patch[i]; 
}

[domain("tri")] 
PSinput DS( HSconstOutput input, float3 UVW : SV_DomainLocation, const OutputPatch<PSinput, 3> quad ) 
{
	PSinput output = (PSinput)0; 
	
	//Convert positions and normals to world space
	output.pos = UVW.x * quad[0].pos + UVW.y * quad[1].pos + UVW.z * quad[2].pos; 
	output.texcoord= UVW.x * quad[0].texcoord + UVW.y * quad[1].texcoord + UVW.z * quad[2].texcoord;
	output.normal = UVW.x * quad[0].normal + UVW.y * quad[1].normal + UVW.z * quad[2].normal;
	output.tangent = UVW.x * quad[0].tangent + UVW.y * quad[1].tangent + UVW.z * quad[2].tangent;
	
	float disp = u_tessScale * texDisp.SampleLevel(texSampler, output.texcoord, 0).r;
	output.pos += (float4(output.normal, 0) * disp);
	
	//Convert positions and normals to viewspace
	output.pos = mul(output.pos, view);
	output.normal = mul(output.normal, (float3x3)view);
	output.tangent = mul(output.tangent, (float3x3)view);
	
	//Convert position to clip space
	output.pos = mul(output.pos, projection);

	return output;
}

float4 PS(PSinput input) : SV_TARGET
{
	//float3 normal = (texNorm.Sample(texSampler, input.texcoord).rgb * 2.0f) - 1;
	//normal = mul(normal, world);
	//normal = normalize(normal);

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	float3 bitangent = normalize(cross(normal, tangent));
	bitangent = normalize(bitangent);
	float3x3 tbn = float3x3(tangent, bitangent, normal); //transforms tangent=>view space
	
	float3 tangentNormal = texNorm.Sample(texSampler, input.texcoord).rgb;
	tangentNormal = normalize(tangentNormal * 2 - 1); //convert 0~1 to -1~+1.
	
	//tbn = transpose(tbn); //inverse matrix - tangent to view
	normal = normalize(mul(tangentNormal ,tbn));
	

	float f = abs(dot(normal, u_lightdir.xyz));
	f = lerp(0.2f, 1.0f, f);

	float4 colour = tex.Sample(texSampler, input.texcoord);
	
	return float4(colour.rgb * f, colour.a);
}
