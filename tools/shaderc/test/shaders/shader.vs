
cbuffer uniforms : register(b0)
{
	float4 u_pos;
	uint   u_resW;
	uint   u_resH;
	float  u_time;
	float  u_tessFactor;
}

struct VSinput
{
	uint vertexID : SV_VertexID;
};

struct PSinput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

PSinput VS(VSinput input)
{
	PSinput output = (PSinput)0;
	
	float2 texcoord = float2( (input.vertexID << 1) & 2, input.vertexID & 2 );
	output.pos = float4( texcoord * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f), 0.0f, 1.0f );
	output.texcoord = texcoord;

	return output;
}
