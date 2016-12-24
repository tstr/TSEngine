/*
	Render test
*/

#include <tsengine.h>
#include <tscore/debug/log.h>
#include <tsgraphics/GraphicsSystem.h>

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////

char g_shaderCode[] = R"(
	Texture2D tex : register(t0);
	SamplerState texSampler : register(s0);
	
	cbuffer uniforms : register(b0)
	{
		float4 u_vector;
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
		//float4 colour = float4(0.7, 0.5, 0.3, 1);
		
		float d = length(u_vector - input.texcoord);
		d = 1.0f - d;
		d = d * d;
		//d = d + d / exp(d);
		//float4 colour = float4(d, d, d, 1.0f);
		
		float4 colour = float4(tex.Sample(texSampler, input.texcoord).rgb * d, 1.0f);
		return colour;
	}
	)";

////////////////////////////////////////////////////////////////////////////////////////////////

class RenderTest : public ts::IApplication
{
private:

	CEngineEnv& m_env;

	size_t frame = 0;
	double timecount = 0.0;

	HBuffer hConstants;
	HTexture hTex;
	HDrawCmd hDraw;
	
	ShaderId vShader; //vertex shader
	ShaderId pShader; //pixel shader

public:
	
	RenderTest(CEngineEnv& env) : m_env(env) {}

	int onInit() override
	{
		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();

		if (!gfx->getShaderManager().createShaderFromString(vShader, g_shaderCode, "VS", eShaderStageVertex))
			return -1;
		if (!gfx->getShaderManager().createShaderFromString(pShader, g_shaderCode, "PS", eShaderStagePixel))
			return -1;

		//Create shader constant buffer
		Vector data;
		SBufferResourceData desc;
		desc.memory = &data;
		desc.size = sizeof(data);
		desc.usage = EBufferType::eBufferTypeConstant;
		if (auto r = api->createResourceBuffer(hConstants, desc))
		{
			tswarn("buffer fail : %", r);
		}

		//Create texture
		STextureResourceData texData;

		Vector texColours[] =
		{
			Vector(1, 0, 0), Vector(0, 1, 0), Vector(0, 0, 1),
			Vector(1, 1, 0), Vector(1, 0, 1), Vector(0, 1, 1),
			Vector(1, 1, 1), Vector(1, 1, 0), Vector(1, 1, 1),
		};

		texData.memory = texColours;
		texData.memoryByteWidth = sizeof(Vector);
		texData.memoryByteDepth = 0;
		STextureResourceDesc texDesc;
		texDesc.depth = 0;
		texDesc.width = 3;
		texDesc.height = 3;
		texDesc.multisampling.count = 1;
		texDesc.texformat = ETextureFormat::eTextureFormatFloat3;
		texDesc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
		texDesc.useMips = false;
		texDesc.textype = ETextureResourceType::eTypeTexture2D;
		texDesc.arraySize = 1;
		if (auto r = api->createResourceTexture(hTex, &texData, texDesc))
		{
			tswarn("texture fail : %", r);
		}

		SDrawCommand drawCmd;

		drawCmd.mode = EDrawMode::eDraw;
		drawCmd.constantBuffers[0] = hConstants;
		drawCmd.vertexCount = 6;
		drawCmd.vertexStart = 0;
		drawCmd.vertexTopology = EVertexTopology::eTopologyTriangleList;
		drawCmd.shaderVertex = gfx->getShaderManager().getShaderHandle(vShader);
		drawCmd.shaderPixel = gfx->getShaderManager().getShaderHandle(pShader);
		//Texture
		drawCmd.textureUnits[0].arrayIndex = 0;
		drawCmd.textureUnits[0].arrayCount = 1;
		drawCmd.textureUnits[0].textureType = eTypeTexture2D;
		drawCmd.textureUnits[0].texture = hTex;
		//Texture sampler
		drawCmd.textureSamplers[0].addressU = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].addressV = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].addressW = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].filtering = eTextureFilterPoint;
		drawCmd.textureSamplers[0].enabled = true;
		//Render states
		drawCmd.depthState.enable = false;
		drawCmd.blendState.enable = false;
		drawCmd.rasterState.enableScissor = false;

		if (auto r = api->createDrawCommand(hDraw, drawCmd))
		{
			tswarn("draw cmd fail : %", r);
		}

		return 0;
	}

	void onUpdate(double dt) override
	{
		//timecount += dt;
		//tsprofile("% : %s", frame, timecount);
		//frame++;

		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();
		IRenderContext* context = gfx->getContext();

		HTarget target;
		api->getDisplayTarget(target);

		SGraphicsSystemConfig config;
		gfx->getConfiguration(config);

		SViewport port;
		port.w = config.width;
		port.h = config.height;

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

		context->bufferUpdate(hConstants, &vec);
		context->draw(target, port, SViewport(), hDraw);
		
		context->finish();
	}

	void onExit() override
	{
		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();

		api->destroyBuffer(hConstants);
		api->destroyTexture(hTex);
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
