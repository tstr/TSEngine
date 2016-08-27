/*
	Application definition
*/

#include <iostream>
#include "application.h"

#include <tsgraphics/rendermodule.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/strings.h>

using namespace ts;
using namespace std;

static void printsysteminfo();

/////////////////////////////////////////////////////////////////////////////////////////////////

void Application::onInit(CEngineSystem* system)
{
	m_system = system;
	printsysteminfo();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	char code[] = R"(
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

	ERenderStatus status = eOk;

	auto api = m_system->getRenderModule()->getApi();

	api->createContext(&m_context);

	bool shader_debug = true;

	SShaderCompileConfig vsconfig;
	vsconfig.debuginfo = shader_debug;
	vsconfig.entrypoint = "VS";
	vsconfig.stage = EShaderStage::eShaderStageVertex;
	tsassert((m_system->getRenderModule()->getShaderManager().compileAndLoadShader(m_vertexshader, code, vsconfig)));

	SShaderCompileConfig psconfig;
	psconfig.debuginfo = shader_debug;
	psconfig.entrypoint = "PS";
	psconfig.stage = EShaderStage::eShaderStagePixel;
	tsassert((m_system->getRenderModule()->getShaderManager().compileAndLoadShader(m_pixelshader, code, psconfig)));

	//Test uniform buffer
	ResourceProxy uniformbuffer;
	SBufferResourceData bufdata;
	Vector v;
	bufdata.memory = &v;
	bufdata.size = sizeof(Vector);
	bufdata.usage = EBufferUsage::eUsageUniform;
	status = api->createResourceBuffer(uniformbuffer, bufdata);
	tsassert(!status);

	tsassert(m_system->getRenderModule()->getTextureManager().loadTexture2D("testimage.png", m_tex2D));

	//Test texture sampler
	ResourceProxy texsampler;
	STextureSamplerDescriptor sampledesc;
	sampledesc.addressU = ETextureAddressMode::eTextureAddressClamp;
	sampledesc.addressV = ETextureAddressMode::eTextureAddressClamp;
	sampledesc.addressW = ETextureAddressMode::eTextureAddressClamp;
	sampledesc.filtering = eTextureFilterBilinear;

	status = api->createTextureSampler(texsampler, sampledesc);
	tsassert(!status);

	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);

	//Fill out render command
	ResourceProxy defaultrendertarget;
	api->getWindowRenderTarget(defaultrendertarget);
	m_command.renderTarget[0] = defaultrendertarget;
	m_command.vertexCount = 6;
	m_command.vertexStart = 0;
	m_command.vertexTopology = EVertexTopology::eTopologyTriangleList;
	m_command.uniformBuffers[0] = uniformbuffer;
	m_command.shaders.stageVertex = m_vertexshader.getShader();
	m_command.shaders.stagePixel = m_pixelshader.getShader();
	m_command.viewport.w = rendercfg.width;
	m_command.viewport.h = rendercfg.height;
	m_command.textureSamplers[0] = texsampler;
	m_command.textures[0] = m_tex2D.getView();

	/////////////////////////////////////////////////////////////////////////////////////////////////
}

void Application::onUpdate(double dt)
{
	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);

	POINT p;
	GetCursorPos(&p);
	Vector vec;
	ScreenToClient((HWND)rendercfg.windowHandle, &p);
	vec.x() = (float)p.x;
	vec.y() = (float)p.y;
	vec.x() /= rendercfg.width;
	vec.y() /= rendercfg.height;
	vec.x() = max(min(vec.x(), 1), 0);
	vec.y() = max(min(vec.y(), 1), 0);

	m_context->resourceBufferUpdate(m_command.uniformBuffers[0], (const void*)&vec);

	m_context->execute(m_command);

	m_system->getRenderModule()->getApi()->executeContext(m_context);
}

void Application::onExit()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////


static void printsysteminfo()
{
	//CPU information
	SSystemInfo inf;
	getSystemInformation(inf);

	tsinfo("CPU name: %", inf.cpuName);
	tsinfo("CPU cores: %", inf.cpuProcessorCount);

	switch (inf.cpuArchitecture)
	{
	case ECPUArchitecture::eCPUarchAMD64:
		tsinfo("CPU architecture: %", "AMD64");
		break;
	case ECPUArchitecture::eCPUarchX86:
		tsinfo("CPU architecture: %", "X86");
		break;
	case ECPUArchitecture::eCPUarchARM:
		tsinfo("CPU architecture: %", "ARM");
		break;
	default:
		tsinfo("CPU architecture: unknown");
	}

	switch (inf.cpuVendorID)
	{
	case ECpuVendorID::eCpuAMD:
		tsinfo("CPU vendor: %", "AMD");
		break;
	case ECpuVendorID::eCpuIntel:
		tsinfo("CPU vendor: %", "Intel");
		break;
	default:
		tsinfo("CPU vendor: unknown");
	}

	//Other
	tsinfo("No. displays: %", inf.numDisplays);
	tsinfo("OS name: %", inf.osName);

	//Memory information
	SSystemMemoryInfo meminf;
	getSystemMemoryInformation(meminf);

	tsinfo("Memory capacity: %MB", (meminf.mCapacity / (1024ui64 * 1024ui64)));

	tsinfo("");
	tsinfo("Hello %", inf.userName);
}

/////////////////////////////////////////////////////////////////////////////////////////////////