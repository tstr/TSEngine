

struct VSInput
{
    uint id : SV_VertexID;
};

struct PSInput
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD;
};

[stage("vertex")]
PSInput VS(VSInput input)
{
	PSInput output = (PSInput)0;
    
    output.texcoord = float2((input.id << 1) & 2, input.id & 2);
    output.pos = float4(output.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
	return output;
}

[stage("pixel")]
float4 PS(PSInput input) : SV_Target0
{
    float f = length(input.texcoord - float2(0.5, 0.5));
    return float4(0.0f, 1.0f, 0.5f, 1.0f);
    //return float4(0, 0, f, f);
}