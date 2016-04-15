/*
	Postprocess effect functions
*/

#ifndef _FX_POST_
#define _FX_POST_

float4 EmbossShaderFunction3x3(Texture2D tex, SamplerState texSampler, float2 texcoords)
{
	float w = scene.resolutionW;
	float h = scene.resolutionH;

	float4 s22 = tex.Sample(texSampler, texcoords); // center
	float4 s11 = tex.Sample(texSampler, texcoords + float2(-1.0f / w, -1.0f / h));
	float4 s33 = tex.Sample(texSampler, texcoords + float2(1.0f / w, 1.0f / h));

	s11.rgb = (s11.r + s11.g + s11.b);
	s22.rgb = (s22.r + s22.g + s22.b) * -0.5;
	s33.rgb = (s22.r + s22.g + s22.b) * 0.2;

	return (s11 + s22 + s33);
	// return col * (s11 + s22 + s33); // with color
}

// Outputs edges only using a A 3x3 edge filter kernel
float4 OutlinesFunction3x3(Texture2D tex, SamplerState texSampler, float2 texcoords)
{
	float w = scene.resolutionW;
	float h = scene.resolutionH;

	float4 lum = float4(0.30, 0.59, 0.11, 1);

	// TOP ROW
	float s11 = dot(tex.Sample(texSampler, texcoords + float2(-1.0f / w, -1.0f / h)), lum);   // LEFT
	float s12 = dot(tex.Sample(texSampler, texcoords + float2(0, -1.0f / h)), lum);             // MIDDLE
	float s13 = dot(tex.Sample(texSampler, texcoords + float2(1.0f / w, -1.0f / h)), lum);    // RIGHT

	// MIDDLE ROW
	float s21 = dot(tex.Sample(texSampler, texcoords + float2(-1.0f / w, 0)), lum);                // LEFT
	// Omit center
	float s23 = dot(tex.Sample(texSampler, texcoords + float2(-1.0f / w, 0)), lum);                // RIGHT

	// LAST ROW
	float s31 = dot(tex.Sample(texSampler, texcoords + float2(-1.0f / w, 1.0f / h)), lum);    // LEFT
	float s32 = dot(tex.Sample(texSampler, texcoords + float2(0, 1.0f / h)), lum);              // MIDDLE
	float s33 = dot(tex.Sample(texSampler, texcoords + float2(1.0f / w, 1.0f / h)), lum); // RIGHT

	// Filter ... thanks internet <span class="wp-smiley wp-emoji wp-emoji-smile" title=":)">:)</span>
	float t1 = s13 + s33 + (2 * s23) - s11 - (2 * s21) - s31;
	float t2 = s31 + (2 * s32) + s33 - s11 - (2 * s12) - s13;

	float4 col;

	if (((t1 * t1) + (t2 * t2)) > 0.05)
	{
		col = float4(0,0,0,1);
	}
	else
	{
		col = float4(1,1,1,1);
	}

	return col;
}

#endif