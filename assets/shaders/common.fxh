/*
	Common shader types and functions
*/


struct SScene
{
	matrix world;
	matrix invWorld;
	matrix view;
	matrix invView;
	matrix projection;
	matrix invProjection;

	float4 lightPos;
	float4 lightColour;
	float4 globalAmbientColour;
	float4 eyePos;
	
	float nearplane;
	float farplane;
	
	float lightConstantAttenuation;
	float lightLinearAttenuation;
	float lightQuadraticAttenuation;
};

struct SMaterial
{
	float4 diffuseColour;
	float4 ambientColour;
	float4 specularColour;
	float4 emissiveColour;
	float specularPower;
	
	uint useDiffuseMap;
	uint useNormalMap;
	uint useDisplacementMap;
	uint useSpecularMap;
};