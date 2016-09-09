/*
	Application definition
*/

#include <iostream>
#include "application.h"

#include <tscore/strings.h>
#include <tscore/delegate.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#include "helpers/geometry.h"
#include "helpers/appinfo.h"

#include "scene/camera.h"

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////
//Window/Input event handlers
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

int Application::onKeyDown(EKeyCode code)
{
	if (code == EKeyCode::eKeyEsc)
	m_system->shutdown();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Application event handlers
/////////////////////////////////////////////////////////////////////////////////////////////////
void Application::onInit(CEngineSystem* system)
{
	m_system = system;
	
	//Print basic information
	printRepositoryInfo();
	printSystemInfo();

	//Register event listeners
	m_system->getInputModule()->addEventListener(this);
	m_system->getWindow()->addEventListener(this);

	m_system->getInputModule()->showCursor(false);
	
	m_camera.reset(new CCamera(m_system->getInputModule()));

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
		
		float3 u_lightdirection;
		float3 u_eyeposition;
	}
	
	struct VSinput
	{
		float4 pos : POSITION;
		float4 colour : COLOUR;
		float2 texcoord : TEXCOORD;
		float3 normal : NORMAL;
	};

	struct PSinput
	{
		float4 pos : SV_POSITION;
		float4 colour : COLOUR;
		float2 texcoord : TEXCOORD;
		
		float3 vnormal : VIEW_NORMAL;
		float4 vpos : VIEW_POSITION;
		float3 ldir : LIGHT_DIR;
		float3 vdir : VIEW_DIR;
	};

	PSinput VS(VSinput input)
	{
		PSinput output = (PSinput)0;
		
		output.pos = input.pos;
		output.pos.w = 1.0f;
		
		output.vnormal = input.normal;
		
		output.pos = mul(output.pos, u_world);
		output.pos = mul(output.pos, u_view);
		output.vpos = output.pos;
		output.pos = mul(output.pos, u_projection);

		output.colour = input.colour;
		output.texcoord = input.texcoord;
		
		output.vnormal = mul(output.vnormal, u_world);
		output.vnormal = mul(output.vnormal, u_view);

		output.ldir = u_lightdirection;
		output.ldir = mul(output.ldir, u_view);

		return output;
	}
	
	float4 PS(PSinput input) : SV_TARGET
	{	
		float4 colour = float4(0.6f, 0.7f, 0.3f, 1.0f);

		float diffuseIntensity = dot(input.vnormal, input.ldir);

		float3 ambient = float3(0.23f, 0.22f, 0.23f);
		float3 diffuse = float3(1.0f, 0.95f, 1.0f) * diffuseIntensity;
		
		return (colour * float4((ambient + diffuse), 1.0f));

		//return float4(tex.Sample(texSampler, input.texcoord).rgb, 1.0f) * factor;
	}
	)";

	ERenderStatus status = eOk;

	auto api = m_system->getRenderModule()->getApi();

	api->createContext(&m_context);

	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Shaders
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
	//Assets
	/////////////////////////////////////////////////////////////////////////////////////////////////

	Path modelfile(rendercfg.rootpath);
	modelfile.addDirectories("cube.tsm");
	m_model.import(modelfile);

	m_model.getVertexBuffer(0, m_system->getRenderModule(), m_vertexBuffer);
	m_model.getIndexBuffer(0, m_system->getRenderModule(), m_indexBuffer);

	SModelMesh mesh = m_model.getMesh(0);

	/////////////////////////////////////////////////////////////////////////////////////////////////

#define VECTOR_OFFSET(idx) (uint32)(idx * sizeof(Vector))

	SShaderInputDescriptor inputdescriptor[4];
	inputdescriptor[0].bufferSlot = 0;
	inputdescriptor[0].byteOffset = VECTOR_OFFSET(0);
	inputdescriptor[0].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[0].semanticName = "POSITION";
	inputdescriptor[0].type = eShaderInputFloat4;

	inputdescriptor[1].bufferSlot = 0;
	inputdescriptor[1].byteOffset = VECTOR_OFFSET(2);
	inputdescriptor[1].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[1].semanticName = "COLOUR";
	inputdescriptor[1].type = eShaderInputFloat4;

	inputdescriptor[2].bufferSlot = 0;
	inputdescriptor[2].byteOffset = VECTOR_OFFSET(1);
	inputdescriptor[2].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[2].semanticName = "TEXCOORD";
	inputdescriptor[2].type = eShaderInputFloat2;

	inputdescriptor[3].bufferSlot = 0;
	inputdescriptor[3].byteOffset = VECTOR_OFFSET(3);
	inputdescriptor[3].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[3].semanticName = "NORMAL";
	inputdescriptor[3].type = eShaderInputFloat3;

	status = api->createShaderInputDescriptor(m_vertexInput, m_vertexshader.getShader(), inputdescriptor, ARRAYSIZE(inputdescriptor));
	tsassert(!status);

	/////////////////////////////////////////////////////////////////////////////////////////////////

	//Test uniform buffer
	m_uniformBuffer = CUniformBuffer(m_system->getRenderModule(), m_uniforms);

	tsassert(m_system->getRenderModule()->getTextureManager().loadTexture2D("cubetexture.png", m_tex2D));

	//Test texture sampler
	STextureSamplerDescriptor sampledesc;
	sampledesc.addressU = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.addressV = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.addressW = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.filtering = eTextureFilterAnisotropic16x;

	status = api->createTextureSampler(m_texSampler, sampledesc);
	tsassert(!status);
	
	//Depth target view
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
	status = api->createViewDepthTarget(m_depthTarget, depthtargetrsc, depthviewdesc);
	tsassert(!status);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}

void Application::onUpdate(double dt)
{
	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);

	//Update camera
	m_camera->setAspectRatio((float)rendercfg.width / rendercfg.height);
	m_camera->update(dt);
	
	//Set uniforms
	m_uniforms.u_world = Matrix::translation(Vector(0.0f, 0.5f, 3.0f));
	m_uniforms.u_view = m_camera->getViewMatrix();
	m_uniforms.u_projection = m_camera->getProjectionMatrix();

	m_uniforms.u_lightdirection = Vector(0.1f, 0.4f, -0.4f); //Light direction
	m_uniforms.u_lightdirection = Matrix::transform(m_uniforms.u_lightdirection, Matrix::fromYawPitchRoll(Vector(0.1f, -0.4f, 0)));// Matrix::rotationY((float)angle));
	m_uniforms.u_lightdirection.normalize();
	
	Matrix::transpose(m_uniforms.u_world);
	Matrix::transpose(m_uniforms.u_view);
	Matrix::transpose(m_uniforms.u_projection);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	SRenderCommand command;
	
	//Clear the depth buffer
	m_context->clearDepthTarget(m_depthTarget, 1.0f);
	
	ResourceProxy defaultrendertarget;
	m_system->getRenderModule()->getApi()->getWindowRenderTarget(defaultrendertarget);

	//Fill out render command
	command.renderTarget[0] = defaultrendertarget;
	command.depthTarget = m_depthTarget;
	command.viewport.w = rendercfg.width;
	command.viewport.h = rendercfg.height;
	command.viewport.x = 0;
	command.viewport.y = 0;
	
	command.uniformBuffers[0] = m_uniformBuffer.getBuffer();
	command.shaders.stageVertex = m_vertexshader.getShader();
	command.shaders.stagePixel = m_pixelshader.getShader();

	command.textureSamplers[0] = m_texSampler;
	command.textures[0] = m_tex2D.getView();

	command.vertexTopology = EVertexTopology::eTopologyTriangleList;
	command.vertexInputDescriptor = m_vertexInput;
	command.vertexBuffer = m_vertexBuffer.getBuffer();
	command.indexBuffer = m_indexBuffer.getBuffer();
	
	command.indexStart = 0;
	command.indexCount = m_indexBuffer.getIndexCount();
	command.vertexStride = m_vertexBuffer.getVertexStride();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	//Update uniforms
	m_context->resourceBufferUpdate(m_uniformBuffer.getBuffer(), (const void*)&m_uniforms);
	//Execute draw call
	m_context->execute(command);
	//Mark context as finished
	m_context->finish();
	//Execute context
	m_system->getRenderModule()->getApi()->executeContext(m_context);
	
}

void Application::onExit()
{
	m_system->getRenderModule()->getApi()->destroyContext(m_context);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
