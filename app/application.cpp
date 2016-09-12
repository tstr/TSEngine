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
#include "scene/modelimporter.h"

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////
//Ctor/dtor
/////////////////////////////////////////////////////////////////////////////////////////////////

Application::Application() {}

Application::~Application() {}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Window/Input event handlers
/////////////////////////////////////////////////////////////////////////////////////////////////

int Application::onWindowEvent(const SWindowEventArgs& args)
{
	if (args.eventcode == EWindowEvent::eEventResize)
	{
		//Recreate the depth target
		buildDepthTarget();
	}
	return 0;
}

int Application::onKeyDown(EKeyCode code)
{
	if (code == EKeyCode::eKeyEsc)
		m_system->shutdown();

	return 0;
}

inline string getMouseCodeName(EMouseButtons button)
{
	switch (button)
	{
	case(EMouseButtons::eMouseButtonLeft): { return "Mouse Left"; }
	case(EMouseButtons::eMouseButtonRight): { return "Mouse Right"; }
	case(EMouseButtons::eMouseButtonMiddle): { return "Mouse Middle"; }
	case(EMouseButtons::eMouseXbutton1): { return "Mouse 1"; }
	case(EMouseButtons::eMouseXbutton2): { return "Mouse 2"; }
	}

	return "";
}

int Application::onMouseDown(const SInputMouseEvent& args)
{
	return 0;
}

int Application::onMouseUp(const SInputMouseEvent& args)
{
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

	//m_system->getInputModule()->showCursor(false);
	
	m_camera.reset(new CCamera(m_system->getInputModule()));
	m_camera->setPosition(Vector(0, 1.0f, 0));

	/////////////////////////////////////////////////////////////////////////////////////////////////

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
	tsassert((m_system->getRenderModule()->getShaderManager().compileAndLoadShaderFile(m_vertexshader, "standard", vsconfig)));

	SShaderCompileConfig psconfig;
	psconfig.debuginfo = shader_debug;
	psconfig.entrypoint = "PS";
	psconfig.stage = EShaderStage::eShaderStagePixel;
	tsassert((m_system->getRenderModule()->getShaderManager().compileAndLoadShaderFile(m_pixelshader, "standard", psconfig)));

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Assets
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	Path modelfile(rendercfg.rootpath);
	modelfile.addDirectories("sponza/sponza.tsm");

	m_model.reset(new CModel(m_system->getRenderModule(), modelfile));

	uint8 vertexAttribs = 0;
	for (uint x = 0; x < m_model->getMeshCount(); x++)
	{
		vertexAttribs |= m_model->getMesh(x).vertexAttributes;
	}
	
	m_materialBuffer = CUniformBuffer(m_system->getRenderModule(), SMaterial::SParams());
	m_sceneBuffer = CUniformBuffer(m_system->getRenderModule(), m_uniforms);

	/////////////////////////////////////////////////////////////////////////////////////////////////

#define VECTOR_OFFSET(idx) (uint32)(idx * sizeof(Vector))

	SShaderInputDescriptor inputdescriptor[6];

	//Position attribute
	inputdescriptor[0].bufferSlot = 0;
	inputdescriptor[0].byteOffset = VECTOR_OFFSET(0);
	inputdescriptor[0].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[0].semanticName = "POSITION";
	inputdescriptor[0].type = eShaderInputFloat4;

	//Texcoord attribute
	inputdescriptor[1].bufferSlot = 0;
	inputdescriptor[1].byteOffset = VECTOR_OFFSET(1);
	inputdescriptor[1].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[1].semanticName = "TEXCOORD";
	inputdescriptor[1].type = eShaderInputFloat2;

	//Colour attribute
	inputdescriptor[2].bufferSlot = 0;
	inputdescriptor[2].byteOffset = VECTOR_OFFSET(2);
	inputdescriptor[2].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[2].semanticName = "COLOUR";
	inputdescriptor[2].type = eShaderInputFloat4;

	//Normal attribute
	inputdescriptor[3].bufferSlot = 0;
	inputdescriptor[3].byteOffset = VECTOR_OFFSET(3);
	inputdescriptor[3].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[3].semanticName = "NORMAL";
	inputdescriptor[3].type = eShaderInputFloat3;

	//Tangent attribute
	inputdescriptor[4].bufferSlot = 0;
	inputdescriptor[4].byteOffset = VECTOR_OFFSET(4);
	inputdescriptor[4].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[4].semanticName = "TANGENT";
	inputdescriptor[4].type = eShaderInputFloat3;

	//Bitangent attribute
	inputdescriptor[5].bufferSlot = 0;
	inputdescriptor[5].byteOffset = VECTOR_OFFSET(5);
	inputdescriptor[5].channel = EShaderInputChannel::eInputPerVertex;
	inputdescriptor[5].semanticName = "BITANGENT";
	inputdescriptor[5].type = eShaderInputFloat3;

	status = api->createShaderInputDescriptor(m_vertexInput, m_vertexshader.getShader(), inputdescriptor, ARRAYSIZE(inputdescriptor));
	tsassert(!status);

	/////////////////////////////////////////////////////////////////////////////////////////////////

	//Test texture sampler
	STextureSamplerDescriptor sampledesc;
	sampledesc.addressU = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.addressV = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.addressW = ETextureAddressMode::eTextureAddressWrap;
	sampledesc.filtering = eTextureFilterAnisotropic16x;

	status = api->createTextureSampler(m_texSampler, sampledesc);
	tsassert(!status);
	
	//Depth target view
	buildDepthTarget();

	/////////////////////////////////////////////////////////////////////////////////////////////////
}

void Application::onUpdate(double dt)
{
	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);

	//Update camera
	m_camera->setAspectRatio((float)rendercfg.width / rendercfg.height);
	m_camera->update(dt);

	float scale = 0.1f;

	//Update displacement of light source
	m_pulsatance += (float)(((2.0 * Pi) / 5.0) * dt);
	Vector lightpos;
	lightpos.x() = 12.0f * sin(m_pulsatance = fmod(m_pulsatance, 2 * Pi));
	//lightpos.y() = 4.0f + 2.0f * sin(m_pulsatance * 2.0f);
	lightpos.y() = 2.0f;
	lightpos.z() = 0.0f;
	lightpos = lightpos / scale;
	
	//Set uniforms
	m_uniforms.world = Matrix::scale(scale, scale, scale);
	m_uniforms.view = m_camera->getViewMatrix();
	m_uniforms.projection = m_camera->getProjectionMatrix();

	m_uniforms.lightPos = lightpos;
	m_uniforms.eyePos = m_camera->getPosition();

	m_uniforms.lightConstantAttenuation = 1.0f;
	m_uniforms.lightLinearAttenuation = 0.04f;
	m_uniforms.lightQuadraticAttenuation = 0.0f;

	//m_uniforms.u_lightdirection = Vector(0.1f, 0.4f, -0.4f); //Light direction
	//m_uniforms.u_lightdirection = Matrix::transform(m_uniforms.u_lightdirection, Matrix::fromYawPitchRoll(Vector(0.1f, -0.4f, 0)));// Matrix::rotationY((float)angle));
	//m_uniforms.u_lightdirection.normalize();

	m_uniforms.init();

	//Update uniforms
	m_context->resourceBufferUpdate(m_sceneBuffer.getBuffer(), (const void*)&m_uniforms);
	
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
	
	command.uniformBuffers[0] = m_sceneBuffer.getBuffer();
	command.uniformBuffers[1] = m_materialBuffer.getBuffer();
	command.shaders.stageVertex = m_vertexshader.getShader();
	command.shaders.stagePixel = m_pixelshader.getShader();

	command.textureSamplers[0] = m_texSampler;

	CVertexBuffer vbuf;
	CIndexBuffer ibuf;
	m_model->getVertexBuffer(vbuf);
	m_model->getIndexBuffer(ibuf);

	command.vertexBuffer = vbuf.getBuffer();
	command.indexBuffer = ibuf.getBuffer();
	command.vertexStride = vbuf.getVertexStride();

	//command.vertexTopology = EVertexTopology::eTopologyTriangleList;
	command.vertexTopology = EVertexTopology::eTopologyTriangleList;
	command.vertexInputDescriptor = m_vertexInput;

	for (uint i = 0; i < m_model->getMeshCount(); i++)
	{
		const SMesh& mesh = m_model->getMesh(i);

		command.textures[0] = mesh.material.diffuseMap.getView();
		command.textures[1] = mesh.material.normalMap.getView();
		command.textures[2] = mesh.material.specularMap.getView();
		command.textures[3] = mesh.material.displacementMap.getView();
		
		command.indexStart = mesh.indexOffset;
		command.indexCount = mesh.indexCount;
		command.vertexBase = mesh.vertexBase;
		
		m_context->resourceBufferUpdate(m_materialBuffer.getBuffer(), (const void*)&mesh.material.params);
		
		//Execute draw call
		m_context->execute(command);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////

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

void Application::buildDepthTarget()
{
	m_depthTarget.reset(nullptr);

	SRenderModuleConfiguration rendercfg;
	m_system->getRenderModule()->getConfiguration(rendercfg);
	ERenderStatus status = eOk;
	auto api = m_system->getRenderModule()->getApi();

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
}

/////////////////////////////////////////////////////////////////////////////////////////////////