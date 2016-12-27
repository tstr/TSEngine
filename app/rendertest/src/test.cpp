/*
	Render test
*/

#include <tsengine.h>
#include <tscore/debug/log.h>
#include <tsgraphics/GraphicsSystem.h>
#include <tsengine/input/inputmodule.h>

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////

void generateCubeMesh(Vector halfextents, vector<Index>& indices, vector<Vector>& vertices, vector<Vector>& texcoords, vector<Vector>& normals);

char g_cubeShaderCode[] = R"(
	//TextureCube tex : register(t0);
	Texture2D tex : register(t0);
	Texture2D texDisp : register(t1);
	SamplerState texSampler : register(s0);

	cbuffer uniforms : register(b0)
	{
		matrix view;
		matrix projection;
		uint   u_resW;
		uint   u_resH;
		float  u_time;
		float  u_tessFactor;
	}

	struct VSinput
	{
		float4 pos : POSITION;
		float3 normal : NORMAL;
		float2 texcoord : TEXCOORD;
		matrix world : WORLD;
	};
	
	struct PSinput
	{
		float4 pos : SV_POSITION;
		float3 normal : NORMAL;
		float2 texcoord : TEXCOORD;
	};
	
	PSinput VS(VSinput input)
	{
		PSinput output = (PSinput)0;
		input.pos.w = 1;
		output.pos = mul(input.pos, input.world);
		output.normal = mul(input.normal, (float3x3)input.world);
		output.texcoord = input.texcoord;
		return output;
	}

	float4 PS(PSinput input) : SV_TARGET
	{
		//float4 colour = tex.Sample(texSampler, input.cubepos);
		float4 colour = tex.Sample(texSampler, input.texcoord);
		return colour;
		//return colour * float4((input.pos.z / input.pos.w).xxx, 1);
	}
	
	struct HSconstOutput 
	{
		float edges[3] : SV_TessFactor;
		float inside : SV_InsideTessFactor;
	};

	HSconstOutput ConstantHS( InputPatch<PSinput, 3> ip, uint PatchID : SV_PrimitiveID ) 
	{
		HSconstOutput output; 
		output.edges[0] = u_tessFactor;
		output.edges[1] = u_tessFactor;
		output.edges[2]= u_tessFactor; 
		output.inside = u_tessFactor;
		return output; 
	}
	
	[domain("tri")] 
	[partitioning("integer")]
	[outputtopology("triangle_cw")]		//triangle topology with clockwise winding
	[outputcontrolpoints(3)] 
	[patchconstantfunc("ConstantHS")]
	PSinput HS( InputPatch<PSinput, 3> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID ) 
	{ 
		PSinput output; 
		output = p[i];
		return output; 
	}

	[domain("tri")] 
	PSinput DS( HSconstOutput input, float3 UVW : SV_DomainLocation, const OutputPatch<PSinput, 3> quad ) 
	{
		PSinput output; 
		
		//Convert positions and normals to world space
		output.pos = UVW.x * quad[0].pos + UVW.y * quad[1].pos + UVW.z * quad[2].pos; 
		output.texcoord= UVW.x * quad[0].texcoord + UVW.y * quad[1].texcoord + UVW.z * quad[2].texcoord;
		output.normal = UVW.x * quad[0].normal + UVW.y * quad[1].normal + UVW.z * quad[2].normal;

		float disp = texDisp.SampleLevel(texSampler, output.texcoord, 0).r;
		output.pos += (float4(-output.normal, 0) * disp * 0.2);

		//Convert positions and normals to viewspace
		output.pos = mul(output.pos, view);
		output.normal = mul(output.normal, (float3x3)view);

		//Convert position to clip space
		output.pos = mul(output.pos, projection);

		return output;
	}
	
	)";

char g_quadShaderCode[] = R"(
	Texture2D tex : register(t0);
	SamplerState texSampler : register(s0);
	
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
	
	float4 PS(PSinput input) : SV_TARGET
	{
		float f = length(input.texcoord - float2(0.5, 0.5));
		return float4(0, 0, f, f);
	}
	)";

////////////////////////////////////////////////////////////////////////////////////////////////

class RenderTest :
	public ts::IApplication,
	public IInputEventListener
{
private:

	CEngineEnv& m_env;

	size_t frame = 0;
	double timecount = 0.0;

	HBuffer hConstants;
	HBuffer hVertexBuffer;
	HBuffer hIndexBuffer;
	HBuffer hTexcoordBuffer;
	HBuffer hNormalBuffer;
	HBuffer hMatrixBuffer;
	HTexture hTex;
	HTexture hTexDisp;
	HDrawCmd hDraw;
	HTexture hCube;
	
	ShaderId vShader; //vertex shader
	ShaderId pShader; //pixel shader
	ShaderId dShader;
	ShaderId hShader;

	struct Constants
	{
		Matrix view;
		Matrix projection;
		uint32 resW = 0;
		uint32 resH = 0;
		float time = 0.0f;
		float tessFactor = 1.0f;
	};

public:
	
	RenderTest(CEngineEnv& env) : m_env(env) {}

	int onInit() override
	{
		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();

		//Create shaders
		if (!gfx->getShaderManager().createShaderFromString(vShader, g_cubeShaderCode, "VS", eShaderStageVertex))
			return -1;
		if (!gfx->getShaderManager().createShaderFromString(pShader, g_cubeShaderCode, "PS", eShaderStagePixel))
			return -1;
		if (!gfx->getShaderManager().createShaderFromString(hShader, g_cubeShaderCode, "HS", eShaderStageHull))
			return -1;
		if (!gfx->getShaderManager().createShaderFromString(dShader, g_cubeShaderCode, "DS", eShaderStageDomain))
			return -1;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create buffers
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//Shader constant buffer
		Constants data;
		SBufferResourceData desc;
		desc.memory = &data;
		desc.size = sizeof(data);
		desc.usage = EBufferType::eBufferTypeConstant;
		if (auto r = api->createResourceBuffer(hConstants, desc))
			tswarn("buffer fail : %", r);

		//Vertex buffer
		vector<Index> indices;
		vector<Vector> vertices;
		vector<Vector> texcoords;
		vector<Vector> normals;
		generateCubeMesh(Vector(0.5f, 0.5f, 0.5f), indices, vertices, texcoords, normals);

		//Position vertex buffer
		SBufferResourceData vdesc;
		vdesc.memory = &vertices[0];
		vdesc.size = (uint32)vertices.size() * sizeof(Vector);
		vdesc.usage = eBufferTypeVertex;
		if (auto r = api->createResourceBuffer(hVertexBuffer, vdesc))
			tswarn("buffer fail : %", r);

		//Texcoord vertex buffer
		SBufferResourceData tdesc;
		tdesc.memory = &texcoords[0];
		tdesc.size = (uint32)texcoords.size() * sizeof(Vector);
		tdesc.usage = eBufferTypeVertex;
		if (auto r = api->createResourceBuffer(hTexcoordBuffer, tdesc))
			tswarn("buffer fail : %", r);

		//Normal vertex buffer
		SBufferResourceData ndesc;
		ndesc.memory = &normals[0];
		ndesc.size = (uint32)normals.size() * sizeof(Vector);
		ndesc.usage = eBufferTypeVertex;
		if (auto r = api->createResourceBuffer(hNormalBuffer, ndesc))
			tswarn("buffer fail : %", r);

		//Matrix buffer
		Matrix matrices[4];
		SBufferResourceData mdesc;
		mdesc.memory = matrices;
		mdesc.size = (uint32)sizeof(Matrix) * 4;
		mdesc.usage = eBufferTypeVertex;
		if (auto r = api->createResourceBuffer(hMatrixBuffer, mdesc))
			tswarn("buffer fail : %", r);

		//Index buffer
		SBufferResourceData idesc;
		idesc.memory = &indices[0];
		idesc.size = (uint32)indices.size() * sizeof(Index);
		idesc.usage = eBufferTypeIndex;
		if (auto r = api->createResourceBuffer(hIndexBuffer, idesc))
			tswarn("buffer fail : %", r);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create texture resources
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/*
		Vector texColours[] =
		{
			Vector(1, 0, 0, 1), Vector(0, 1, 0, 1), Vector(0, 0, 1, 1),
			Vector(1, 1, 0, 1), Vector(1, 0, 1, 1), Vector(0, 1, 1, 1),
			Vector(0, 0, 1, 1), Vector(0, 1, 0, 1), Vector(1, 0, 0, 1),
		};

		//Create texture
		STextureResourceData texData;
		texData.memory = texColours;
		texData.memoryByteWidth = sizeof(Vector);
		texData.memoryByteDepth = 0;
		STextureResourceDesc texDesc;
		texDesc.depth = 0;
		texDesc.width = 3;
		texDesc.height = 3;
		texDesc.multisampling.count = 1;
		texDesc.texformat = ETextureFormat::eTextureFormatFloat4;
		texDesc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
		texDesc.useMips = false;
		texDesc.textype = ETextureResourceType::eTypeTexture2D;
		texDesc.arraySize = 1;
		if (auto r = api->createResourceTexture(hTex, &texData, texDesc))
		{
			tswarn("tex fail : %", r);
		}
		//*/

		//*
		CTexture2D tex;
		if (!gfx->getTextureManager().loadTexture2D("cubetexture.png", tex))
		{
			tswarn("tex fail");
		}
		hTex = tex.getHandle();

		CTexture2D texDisp;
		if (!gfx->getTextureManager().loadTexture2D("cubetexture_disp.png", texDisp))
		{
			tswarn("tex fail");
		}
		hTexDisp = texDisp.getHandle();
		//*/

		/*
		Vector cubeColours[6]
		{
			Vector(0, 0, 0, 1),
			Vector(0, 0, 1, 1),
			Vector(0, 1, 0, 1),
			Vector(1, 1, 1, 1),
			Vector(1, 0, 0, 1),
			Vector(1, 0, 1, 1),
		};

		STextureResourceData cubeData[6];

		for (int i = 0; i < 6; i++)
		{
			cubeData[i].memory = &cubeColours[i];
			cubeData[i].memoryByteWidth = sizeof(Vector);
			cubeData[i].memoryByteDepth = 0;
		}

		STextureResourceDesc cubeDesc;
		cubeDesc.depth = 0;
		cubeDesc.width = 1;
		cubeDesc.height = 1;
		cubeDesc.multisampling.count = 1;
		cubeDesc.texformat = ETextureFormat::eTextureFormatFloat4;
		cubeDesc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
		cubeDesc.useMips = false;
		cubeDesc.textype = ETextureResourceType::eTypeTextureCube;
		cubeDesc.arraySize = 6;
		
		if (auto r = api->createResourceTexture(hCube, cubeData, cubeDesc))
			tswarn("cubemap fail : %", r);
		//*/

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create commands
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		SDrawCommand drawCmd;

		drawCmd.mode = EDrawMode::eDrawIndexedInstanced;
		drawCmd.constantBuffers[0] = hConstants;
		//drawCmd.vertexCount = 6;
		drawCmd.vertexStart = 0;
		drawCmd.vertexTopology = eTopologyPatchList3;//EVertexTopology::eTopologyTriangleList;

		drawCmd.shaderVertex = gfx->getShaderManager().getShaderHandle(vShader);
		drawCmd.shaderPixel = gfx->getShaderManager().getShaderHandle(pShader);
		drawCmd.shaderHull = gfx->getShaderManager().getShaderHandle(hShader);
		drawCmd.shaderDomain = gfx->getShaderManager().getShaderHandle(dShader);

		drawCmd.vertexBuffers[0] = hVertexBuffer;
		drawCmd.vertexBuffers[1] = hTexcoordBuffer;
		drawCmd.vertexBuffers[2] = hNormalBuffer;
		drawCmd.indexBuffer = hIndexBuffer;

		drawCmd.vertexOffsets[0] = 0;
		drawCmd.vertexStrides[0] = sizeof(Vector);
		drawCmd.vertexOffsets[1] = 0;
		drawCmd.vertexStrides[1] = sizeof(Vector);
		drawCmd.vertexOffsets[2] = 0;
		drawCmd.vertexStrides[2] = sizeof(Vector);

		drawCmd.instanceCount = 4;
		drawCmd.vertexCount = (uint32)vertices.size();
		drawCmd.indexCount = (uint32)indices.size();
		drawCmd.vertexAttribCount = 4;

		drawCmd.vertexAttribs[0].bufferSlot = 0;
		drawCmd.vertexAttribs[0].byteOffset = 0;
		drawCmd.vertexAttribs[0].channel = EVertexAttributeChannel::eChannelPerVertex;
		drawCmd.vertexAttribs[0].semanticName = "POSITION";
		drawCmd.vertexAttribs[0].type = EVertexAttributeType::eAttribFloat4;
		drawCmd.vertexAttribs[1].bufferSlot = 1;
		drawCmd.vertexAttribs[1].byteOffset = 0;
		drawCmd.vertexAttribs[1].channel = EVertexAttributeChannel::eChannelPerVertex;
		drawCmd.vertexAttribs[1].semanticName = "TEXCOORD";
		drawCmd.vertexAttribs[1].type = EVertexAttributeType::eAttribFloat2;
		drawCmd.vertexAttribs[2].bufferSlot = 2;
		drawCmd.vertexAttribs[2].byteOffset = 0;
		drawCmd.vertexAttribs[2].channel = EVertexAttributeChannel::eChannelPerVertex;
		drawCmd.vertexAttribs[2].semanticName = "NORMAL";
		drawCmd.vertexAttribs[2].type = EVertexAttributeType::eAttribFloat3;
		drawCmd.vertexBuffers[2] = hMatrixBuffer;

		drawCmd.vertexAttribs[3].bufferSlot = 3;
		drawCmd.vertexAttribs[3].byteOffset = 0;
		drawCmd.vertexAttribs[3].channel = EVertexAttributeChannel::eChannelPerInstance;
		drawCmd.vertexAttribs[3].semanticName = "WORLD";
		drawCmd.vertexAttribs[3].type = EVertexAttributeType::eAttribMatrix;
		drawCmd.vertexBuffers[3] = hMatrixBuffer;

		drawCmd.vertexOffsets[3] = 0;
		drawCmd.vertexStrides[3] = sizeof(Matrix);

		//Texture
		drawCmd.textureUnits[0].arrayIndex = 0;
		drawCmd.textureUnits[0].arrayCount = 1;
		//drawCmd.textureUnits[0].textureType = eTypeTextureCube;
		//drawCmd.textureUnits[0].texture = hCube;
		drawCmd.textureUnits[0].textureType = eTypeTexture2D;
		drawCmd.textureUnits[0].texture = hTex;
		drawCmd.textureUnits[1].arrayIndex = 0;
		drawCmd.textureUnits[1].arrayCount = 1;
		drawCmd.textureUnits[1].textureType = eTypeTexture2D;
		drawCmd.textureUnits[1].texture = hTexDisp;
		//Texture sampler
		drawCmd.textureSamplers[0].addressU = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].addressV = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].addressW = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].filtering = eTextureFilterTrilinear;
		drawCmd.textureSamplers[0].enabled = true;
		//Render states
		drawCmd.depthState.enableDepth = true;
		drawCmd.blendState.enable = false;
		drawCmd.rasterState.enableScissor = false;
		drawCmd.rasterState.cullMode = eCullBack;
		drawCmd.rasterState.fillMode = eFillWireframe;

		if (auto r = api->createDrawCommand(hDraw, drawCmd))
			tswarn("draw cmd fail : %", r);
		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		return 0;
	}

	void onUpdate(double dt) override
	{
		timecount += dt;
		tsprofile("% : %s", frame, timecount);
		frame++;

		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();
		IRenderContext* context = gfx->getContext();

		HTarget target;
		api->getDisplayTarget(target);

		SGraphicsSystemConfig config;
		gfx->getConfiguration(config);

		/*
		POINT p;
		GetCursorPos(&p);
		Vector vec;
		ScreenToClient((HWND)config.windowHandle, &p);
		vec.x() = (float)p.x;
		vec.y() = (float)p.y;
		vec.x() /= config.width;
		vec.y() /= config.height;
		vec.x() = max(min(vec.x(), 1), 0);
		vec.y() = max(min(vec.y(), 1), 0);
		*/

		//Update instance matrices
		Vector positions[] = 
		{
			Vector(-2.0f, -2.0f, 5.0f),
			Vector(-2.0f, +2.0f, 5.0f),
			Vector(+2.0f, -2.0f, 5.0f),
			Vector(+2.0f, +2.0f, 5.0f)
		};

		Matrix matrices[4];

		for (int i = 0; i < 4; i++)
		{
			//data.worldViewProj[i] = Matrix::fromYawPitchRoll(positions[i] * (Pi / 4));
			matrices[i] = Matrix::rotationY((float)timecount);
			matrices[i] = matrices[i] * Matrix::translation(positions[i]);
			Matrix::transpose(matrices[i]);
		}
		
		//Update shader constants
		Constants data;
		data.view = Matrix::identity();
		data.projection = Matrix::perspectiveFieldOfView(Pi / 2, (float)config.width / config.height, 0.1f, 10.0f);
		Matrix::transpose(data.view);
		Matrix::transpose(data.projection);
		//data.pos = vec;
		data.resW = config.width;
		data.resH = config.height;
		data.time = (float)timecount;
		/*
		float interp = (float)sin(timecount);
		interp += 1.0f;
		interp /= 2.0f;
		data.tessFactor = ((20.0f - 1.0f) * interp) + 1.0f;
		*/
		data.tessFactor = 12.0f;

		context->bufferUpdate(hMatrixBuffer, matrices);
		context->bufferUpdate(hConstants, &data);
		context->draw(target, SViewport(config.width, config.height, 0, 0), SViewport(), hDraw);
		
		context->finish();
	}

	void onExit() override
	{
		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();

		api->destroyBuffer(hConstants);
		api->destroyTexture(hTex);
		api->destroyTexture(hTexDisp);
		//api->destroyTexture(hCube);
		api->destroyBuffer(hIndexBuffer);
		api->destroyBuffer(hVertexBuffer);
		api->destroyBuffer(hTexcoordBuffer);
		api->destroyBuffer(hNormalBuffer);
		api->destroyBuffer(hMatrixBuffer);
		api->destroyDrawCommand(hDraw);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	//Set startup parameters
	ts::SEngineStartupParams startup;
	startup.argc = argc;
	startup.argv = argv;

#ifdef TS_PLATFORM_WIN32

	startup.appInstance = (void*)GetModuleHandle(0);
	startup.showWindow = SW_SHOWDEFAULT;

	char path[MAX_PATH];
	GetModuleFileNameA((HMODULE)startup.appInstance, path, MAX_PATH);
	startup.appPath = path;

#endif

	//Run engine
	CEngineEnv engine(startup);
	RenderTest test(engine);
	return engine.start(test);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void generateCubeMesh(Vector halfextents, vector<Index>& indices, vector<Vector>& vertices, vector<Vector>& texcoords, vector<Vector>& normals)
{
	float x = halfextents.x();
	float y = halfextents.y();
	float z = halfextents.z();

	/*
	indices = vector<Index>{
		0,2,1,
		0,3,2,

		1,2,6,
		6,5,1,

		4,5,6,
		6,7,4,

		2,3,6,
		6,3,7,

		0,7,3,
		0,4,7,

		0,1,5,
		0,5,4
	};
	*/

	/*
	texcoords = vector<Vector>{

		{ 0, 1 },
		{ 1, 1 },
		{ 1, 0 },
		{ 0, 0 },

		// { 0, 1 },
		// { 1, 1 },
		// { 1, 0 },
		// { 0, 0 },

		{ 1, 1 },
		{ 0, 1 },
		{ 0, 0 },
		{ 1, 0 },
	};
	*/

	/*
	vertices = vector<Vector>{
		{ -x, -y, -z, }, // 0
		{ x, -y, -z, }, // 1
		{ x,  y, -z, }, // 2
		{ -x,  y, -z, }, // 3
		{ -x, -y,  z, }, // 4
		{ x, -y,  z, }, // 5
		{ x,  y,  z, }, // 6
		{ -x,  y,  z }, // 7
	};
	*/
	
	vertices = vector<Vector> {
		Vector(1.99412, -1.98691, 2.00283),
		Vector(1.99412, -1.98691, -1.99717),
		Vector(-2.00588, -1.98691, -1.99717),
		Vector(-2.00588, -1.98691, 2.00283),
		Vector(1.99413, 2.01309, 2.00283),
		Vector(-2.00588, 2.01309, 2.00283),
		Vector(-2.00588, 2.01309, -1.99717),
		Vector(1.99412, 2.01309, -1.99717),
		Vector(1.99412, -1.98691, 2.00283),
		Vector(1.99413, 2.01309, 2.00283),
		Vector(1.99412, 2.01309, -1.99717),
		Vector(1.99412, -1.98691, -1.99717),
		Vector(1.99412, -1.98691, -1.99717),
		Vector(1.99412, 2.01309, -1.99717),
		Vector(-2.00588, 2.01309, -1.99717),
		Vector(-2.00588, -1.98691, -1.99717),
		Vector(-2.00588, -1.98691, -1.99717),
		Vector(-2.00588, 2.01309, -1.99717),
		Vector(-2.00588, 2.01309, 2.00283),
		Vector(-2.00588, -1.98691, 2.00283),
		Vector(1.99413, 2.01309, 2.00283),
		Vector(1.99412, -1.98691, 2.00283),
		Vector(-2.00588, -1.98691, 2.00283),
		Vector(-2.00588, 2.01309, 2.00283),
	};

	for (Vector& v : vertices)
	{
		v = v * Vector(x, y, z);
		v.w() = 1.0f;
	}

	normals = vector<Vector>{
		Vector(0, -1, 0),
		Vector(0, -1, 0),
		Vector(0, -1, 0),
		Vector(0, -1, 0),
		Vector(0, 1, -0),
		Vector(0, 1, -0),
		Vector(0, 1, -0),
		Vector(0, 1, -0),
		Vector(1, 0, -0),
		Vector(1, 0, -0),
		Vector(1, 0, -0),
		Vector(1, 0, -0),
		Vector(-0, -0, -1),
		Vector(-0, -0, -1),
		Vector(-0, -0, -1),
		Vector(-0, -0, -1),
		Vector(-1, -0, 0),
		Vector(-1, -0, 0),
		Vector(-1, -0, 0),
		Vector(-1, -0, 0),
		Vector(0, 0, 1),
		Vector(0, 0, 1),
		Vector(0, 0, 1),
		Vector(0, 0, 1),
	};

	texcoords = vector<Vector>{
		Vector(0, 1),
		Vector(1, 1),
		Vector(1, 0),
		Vector(0, 0),
		Vector(0, 1),
		Vector(1, 1),
		Vector(1, 0),
		Vector(0, 0),
		Vector(0.9999, 0.9999),
		Vector(0.9999, 0.000100017),
		Vector(0.0001, 0.000100017),
		Vector(0.0001, 0.9999),
		Vector(0.9999, 0.9999),
		Vector(0.9999, 0.000100017),
		Vector(0.0001, 0.000100017),
		Vector(0.0001, 0.9999),
		Vector(0, 1),
		Vector(1, 1),
		Vector(1, 0),
		Vector(0, 0),
		Vector(0, 1),
		Vector(1, 1),
		Vector(1, 0),
		Vector(0, 0),
	};

	indices = vector<Index> {
		3,
		2,
		1,
		3,
		1,
		0,
		7,
		6,
		5,
		7,
		5,
		4,
		11,
		10,
		9,
		11,
		9,
		8,
		15,
		14,
		13,
		15,
		13,
		12,
		19,
		18,
		17,
		19,
		17,
		16,
		23,
		22,
		21,
		23,
		21,
		20,
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////
