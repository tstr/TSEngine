/*
	Application definition
*/

#include <iostream>
#include "application.h"

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/strings.h>

#include <tscore/delegate.h>

#include "meshes.h"

using namespace ts;
using namespace std;

static void printsysteminfo();
static void printrepositoryinfo();

/////////////////////////////////////////////////////////////////////////////////////////////////
//Input event handlers
/////////////////////////////////////////////////////////////////////////////////////////////////

enum EActionFlags
{
	eForward = 0x1,
	eBack	 = 0x2,
	eLeft	 = 0x4,
	eRight	 = 0x8,
};

int Application::InputListener::onMouse(int16 dx, int16 dy)
{
	const float sensitivity = 0.0001f;
	const float pi2 = 2 * Pi;
	m_pApp->m_camAngleX = m_pApp->m_camAngleX + pi2 * sensitivity * (float)dx;
	m_pApp->m_camAngleY = m_pApp->m_camAngleY + pi2 * sensitivity * (float)dy;

	m_pApp->m_camAngleX = fmod(m_pApp->m_camAngleX, pi2);
	m_pApp->m_camAngleY = fmod(m_pApp->m_camAngleY, pi2);

	return 0;
}

int Application::InputListener::onKeyDown(EKeyCode code)
{
	if (code == EKeyCode::eKeyEsc)
		m_pApp->m_system->shutdown();

	switch (code)
	{
	case eKeyW: m_pApp->m_actionflags |= eForward; break;
	case eKeyS: m_pApp->m_actionflags |= eBack;	   break;
	case eKeyA: m_pApp->m_actionflags |= eLeft;	   break;
	case eKeyD: m_pApp->m_actionflags |= eRight;   break;
	}

	return 0;
}

int Application::InputListener::onKeyUp(EKeyCode code)
{
	switch (code)
	{
	case eKeyW: m_pApp->m_actionflags &= ~eForward; break;
	case eKeyS: m_pApp->m_actionflags &= ~eBack;	break;
	case eKeyA: m_pApp->m_actionflags &= ~eLeft;	break;
	case eKeyD: m_pApp->m_actionflags &= ~eRight;	break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Window event handler
/////////////////////////////////////////////////////////////////////////////////////////////////

int Application::onWindowEvent(const SWindowEventArgs& args)
{
	auto input = m_system->getInputModule();

	if (args.eventcode == EWindowEvent::eEventSetfocus)
	{
		input->showCursor(false);
	}
	else if (args.eventcode == EWindowEvent::eEventKillfocus)
	{
		input->showCursor(true);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Application event handlers
/////////////////////////////////////////////////////////////////////////////////////////////////
void Application::onInit(CEngineSystem* system)
{
	m_system = system;
	
	//Print basic information
	printrepositoryinfo();
	printsysteminfo();

	m_system->getInputModule()->addEventListener(&m_inputListener);
	m_system->getWindow()->addEventListener(this);

	m_system->getInputModule()->showCursor(false);

	/////////////////////////////////////////////////////////////////////////////////////////////////

	/*
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
	*/

	char cubeshadercode[] = R"(
	Texture2D tex : register(t0);
	SamplerState texSampler : register(s0);
	
	cbuffer uniforms : register(b0)
	{
		matrix u_world;
		matrix u_view;
		matrix u_projection;
		float3 u_direction;
	}
	
	struct VSinput
	{
		float4 pos : POSITION;
		float4 colour : COLOUR;
		float2 texcoord : TEXCOORD;
	};

	struct PSinput
	{
		float4 pos : SV_POSITION;
		float4 colour : COLOUR;
		float2 texcoord : TEXCOORD;
	};

	PSinput VS(VSinput input)
	{
		PSinput output = (PSinput)0;
		output.pos = input.pos;
		output.pos.w = 1.0f;
		
		output.pos = mul(output.pos, u_world);
		output.pos = mul(output.pos, u_view);
		output.pos = mul(output.pos, u_projection);

		output.colour = input.colour;
		output.texcoord = input.texcoord;

		return output;
	}
	
	float4 PS(PSinput input) : SV_TARGET
	{
		return float4(tex.Sample(texSampler, input.texcoord).rgb, 1.0f);
	}
	)";

	ERenderStatus status = eOk;

	auto api = m_system->getRenderModule()->getApi();

	api->createContext(&m_context);

	/////////////////////////////////////////////////////////////////////////////////////////////////

	bool shader_debug = true;
	
	SShaderCompileConfig vsconfig;
	vsconfig.debuginfo = shader_debug;
	vsconfig.entrypoint = "VS";
	vsconfig.stage = EShaderStage::eShaderStageVertex;
	tsassert((m_system->getRenderModule()->getShaderManager().compileAndLoadShader(m_vertexshader, cubeshadercode, vsconfig)));

	SShaderCompileConfig psconfig;
	psconfig.debuginfo = shader_debug;
	psconfig.entrypoint = "PS";
	psconfig.stage = EShaderStage::eShaderStagePixel;
	tsassert((m_system->getRenderModule()->getShaderManager().compileAndLoadShader(m_pixelshader, cubeshadercode, psconfig)));

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Cube mesh
	/////////////////////////////////////////////////////////////////////////////////////////////////

	struct Vertex
	{
		Vector pos;
		Vector colour;
		Vector texcoord;
	};

	vector<Vector> vertexPositions;
	vector<Vector> vertexTexcoords;
	vector<Index> indices;

	generateCubeMesh(Vector(0.5f, 0.5f, 0.5f), indices, vertexPositions, vertexTexcoords);

	vector<Vertex> vertices;
	vertices.resize(vertexPositions.size());

	size_t sz = vertexPositions.size();
	for (size_t i = 0; i < sz; i++)
	{
		vertices[i].pos = vertexPositions[i];
		vertices[i].texcoord = vertexTexcoords[i];
		vertices[i].colour = Vector((float)i / sz, 0.0f, 0.0f, 1.0f);
	}

	m_vertexBuffer = CVertexBuffer(m_system->getRenderModule(), &vertices[0], (uint32)vertices.size());
	m_indexBuffer = CIndexBuffer(m_system->getRenderModule(), &indices[0], (uint32)indices.size());

	SShaderInputDescriptor inputdescriptor[3];
	inputdescriptor[0].bufferSlot = 0;
	inputdescriptor[0].byteOffset = 0;
	inputdescriptor[0].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[0].semanticName = "POSITION";
	inputdescriptor[0].type = eShaderInputFloat4;

	inputdescriptor[1].bufferSlot = 0;
	inputdescriptor[1].byteOffset = sizeof(Vector);
	inputdescriptor[1].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[1].semanticName = "COLOUR";
	inputdescriptor[1].type = eShaderInputFloat4;

	inputdescriptor[2].bufferSlot = 0;
	inputdescriptor[2].byteOffset = sizeof(Vector) * 2;
	inputdescriptor[2].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[2].semanticName = "TEXCOORD";
	inputdescriptor[2].type = eShaderInputFloat2;

	ResourceProxy sid;

	status = api->createShaderInputDescriptor(sid, m_vertexshader.getShader(), inputdescriptor, ARRAYSIZE(inputdescriptor));
	tsassert(!status);

	/////////////////////////////////////////////////////////////////////////////////////////////////

	//Test uniform buffer
	m_uniformBuffer = CUniformBuffer(m_system->getRenderModule(), m_uniforms);

	tsassert(m_system->getRenderModule()->getTextureManager().loadTexture2D("cubetexture.png", m_tex2D));

	//Test texture sampler
	ResourceProxy texsampler;
	STextureSamplerDescriptor sampledesc;
	sampledesc.addressU = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.addressV = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.addressW = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.filtering = eTextureFilterAnisotropic16x;

	status = api->createTextureSampler(texsampler, sampledesc);
	tsassert(!status);

	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);
	 
	//Depth target view
	ResourceProxy depthtarget;
	ResourceProxy depthtargetrsc;
	STextureResourceDescriptor depthdesc;
	depthdesc.height = rendercfg.height;
	depthdesc.width = rendercfg.width;
	depthdesc.texformat = ETextureFormat::eTextureFormatDepth32;
	depthdesc.texmask = eTextureMaskDepthTarget;
	depthdesc.textype = eTypeTexture2D;
	depthdesc.useMips = false;
	depthdesc.multisampling = rendercfg.multisampling;
	status = api->createResourceTexture(depthtargetrsc, nullptr, depthdesc);
	tsassert(!status);
	STextureViewDescriptor depthviewdesc;
	depthviewdesc.arrayCount = 1;
	depthviewdesc.arrayIndex = 0;
	status = api->createViewDepthTarget(depthtarget, depthtargetrsc, depthviewdesc);
	tsassert(!status);

	/////////////////////////////////////////////////////////////////////////////////////////////////

	//Fill out render command
	ResourceProxy defaultrendertarget;
	api->getWindowRenderTarget(defaultrendertarget);
	m_command.renderTarget[0] = defaultrendertarget;
	m_command.depthTarget = depthtarget;
	m_command.vertexCount = 6;
	m_command.vertexStart = 0;
	m_command.vertexTopology = EVertexTopology::eTopologyTriangleList;
	m_command.uniformBuffers[0] = m_uniformBuffer.getBuffer();
	m_command.shaders.stageVertex = m_vertexshader.getShader();
	m_command.shaders.stagePixel = m_pixelshader.getShader();
	m_command.viewport.w = rendercfg.width;
	m_command.viewport.h = rendercfg.height;
	m_command.textureSamplers[0] = texsampler;
	m_command.textures[0] = m_tex2D.getView();

	m_command.vertexBuffer = m_vertexBuffer.getBuffer();
	m_command.indexBuffer = m_indexBuffer.getBuffer();
	m_command.indexCount = (uint32)indices.size();
	m_command.vertexStride = sizeof(Vertex);
	m_command.vertexInputDescriptor = sid;

	/////////////////////////////////////////////////////////////////////////////////////////////////
}

void Application::onUpdate(double dt)
{
	//Clear the depth buffer
	m_context->clearDepthTarget(m_command.depthTarget, 1.0f);

	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);

	POINT p;
	GetCursorPos(&p);
	Vector vec;
	ScreenToClient((HWND)rendercfg.windowHandle, &p);
	vec.x() = (float)p.x / rendercfg.width;
	vec.y() = (float)p.y / rendercfg.height;
	vec.x() = max(min(vec.x(), 1), 0);
	vec.y() = max(min(vec.y(), 1), 0);

	m_command.viewport.w = rendercfg.width;
	m_command.viewport.h = rendercfg.height;

	static double angle = 0.0f;
	angle += (Pi / 8) * dt;
	angle = fmod(angle, 2 * Pi);
	angle = 0.0f;

	//Update position
	int8 actions = m_actionflags.load();
	const float speed = 3.0f;
	float ax = m_camAngleX.load();
	float ay = m_camAngleY.load();
	float dis = speed * (float)dt;

	if (actions & eForward)
	{
		m_camPosition.x() += dis * sin(ax);
		m_camPosition.z() += dis * cos(ax);
	}
	if (actions & eBack)
	{
		m_camPosition.x() -= dis * sin(ax);
		m_camPosition.z() -= dis * cos(ax);
	}
	if (actions & eLeft)
	{
		m_camPosition.x() -= dis * cos(ax);
		m_camPosition.z() += dis * sin(ax);
	}
	if (actions & eRight)
	{
		m_camPosition.x() += dis * cos(ax);
		m_camPosition.z() -= dis * sin(ax);
	}

	//Set uniforms
	m_uniforms.u_world = Matrix::rotationY((float)angle) * Matrix::translation(Vector(0.0f, 0.0f, 3.0f));
	m_uniforms.u_view = (Matrix::rotationX(m_camAngleY) * Matrix::rotationY(m_camAngleX) * Matrix::translation(m_camPosition)).inverse();
	m_uniforms.u_projection = Matrix::perspectiveFieldOfView(Pi / 2, (float)rendercfg.width / rendercfg.height, 0.1f, 20.0f);
	
	Matrix::transpose(m_uniforms.u_world);
	Matrix::transpose(m_uniforms.u_view);
	Matrix::transpose(m_uniforms.u_projection);
	
	//Update uniforms
	m_context->resourceBufferUpdate(m_command.uniformBuffers[0], (const void*)&m_uniforms);

	m_context->execute(m_command);

	m_system->getRenderModule()->getApi()->executeContext(m_context);
}

void Application::onExit()
{
	m_system->getRenderModule()->getApi()->destroyContext(m_context);
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
		tsinfo("CPU architecture: AMD64");
		break;
	case ECPUArchitecture::eCPUarchX86:
		tsinfo("CPU architecture: X86");
		break;
	case ECPUArchitecture::eCPUarchARM:
		tsinfo("CPU architecture: ARM");
		break;
	default:
		tsinfo("CPU architecture: unknown");
	}

	switch (inf.cpuVendorID)
	{
	case ECpuVendorID::eCpuAMD:
		tsinfo("CPU vendor: AMD");
		break;
	case ECpuVendorID::eCpuIntel:
		tsinfo("CPU vendor: Intel");
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

void printrepositoryinfo()
{
	tsinfo("Git refspec: %", TS_GIT_REFSPEC);
	tsinfo("Git commit SHA1: %", TS_GIT_SHA1);
	tsinfo("Git commit date: \"%\"", TS_GIT_COMMIT_DATE);
	tsinfo("Git commit subject: \"%\"", TS_GIT_COMMIT_SUBJECT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
