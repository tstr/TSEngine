/*
	Standard Shader
*/

#include "Common.h"

cbuffer Scene : SLOT_PER_SCENE
{
	SceneParams scene;
};

cbuffer Mesh : SLOT_PER_MESH
{
	MeshParams mesh;
};

Texture2D diffuse : register(t0);
SamplerState texsample : register(s0);

/*
	Vertex Stage
*/
PixelInput_PosTex VS(VertexInput_PosTex input)
{
	PixelInput_PosTex output = (PixelInput_PosTex)0;
	
	//ensure translation is possible
	input.pos.w = 1;
	output.pos = input.pos;
	
	//transform position
	output.pos = mul(output.pos, mesh.world);
	output.pos = mul(output.pos, scene.view);

	//save view position of vertex
	output.posv = output.pos;
	output.pos = mul(output.pos, scene.projection);
	
	output.texcoord = input.texcoord;
	output.texcoord.y = 1.0f - output.texcoord.y;

	return output;
}

/*
	Pixel Stage
*/
float4 PS(PixelInput_PosTex input) : SV_Target0
{
	return diffuse.Sample(texsample, input.texcoord);
}

