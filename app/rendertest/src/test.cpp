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
	
	HTexture hTex		= HTEXTURE_NULL;
	HTexture hTexDisp	= HTEXTURE_NULL;
	HTexture hTexNorm	= HTEXTURE_NULL;
	HTexture hCube		= HTEXTURE_NULL;

	HDrawCmd hDraw;
	
	/*
	ShaderId vShader; //vertex shader
	ShaderId pShader; //pixel shader
	ShaderId dShader;
	ShaderId hShader;
	*/

	struct Constants
	{
		Matrix world;
		Matrix view;
		Matrix projection;
		Vector lightDir;
		float tessScale = 1.0f;
		float tessFactor = 1.0f;
		float time = 0.0f;
	};

	struct Vertex
	{
		Vector position;
		Vector normal;
		Vector texcoord;
		Vector tangent;
	};

public:
	
	RenderTest(CEngineEnv& env) : m_env(env) {}

	int onInit() override
	{
		GraphicsSystem* gfx = m_env.getGraphics();
		IRender* api = gfx->getApi();

		ShaderId programId = 0;
		SShaderProgram program;
		if (EShaderManagerStatus s = gfx->getShaderManager().load("TestCube", programId))
		{
			tserror("Unable to load shader \"TestShader\" : %", s);
			return -1;
		}

		gfx->getShaderManager().getProgram(programId, program);

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

		/*
		//Vertex buffer
		vector<Index> indices;
		vector<Vector> vertices;
		vector<Vector> texcoords;
		vector<Vector> normals;

		generateCubeMesh(Vector(0.5f, 0.5f, 0.5f), indices, vertices, texcoords, normals);

		//Create mesh
		CVertexBuilder vertexBuilder;
		SVertexMesh meshData;
		vertexBuilder.begin((uint32)vertices.size(), (uint32)indices.size());
		vertexBuilder.setAttributeStream("POSITION", vertices,  eAttribFloat3);
		vertexBuilder.setAttributeStream("NORMAL",   normals,   eAttribFloat3);
		vertexBuilder.setAttributeStream("TEXCOORD", texcoords, eAttribFloat2);
		vertexBuilder.setAttributeStream("TANGENT",  normals,	eAttribFloat3);
		vertexBuilder.setIndexStream(indices);
		vertexBuilder.end(meshData);

		meshData.vertexTopology = eTopologyTriangleList;

		//*/

		//*
		Vector attributePositions[] =
		{
			Vector(-1, -1, 0), //bottom left
			Vector(-1, +1, 0), //top left
			Vector(+1, +1, 0), //top right
			Vector(+1, -1, 0), //bottom right
		};

		Vector attributeNormals[] =
		{
			Vector(0, 0, -1),
			Vector(0, 0, -1),
			Vector(0, 0, -1),
			Vector(0, 0, -1),
		};

		Vector attributeTexcoords[] =
		{
			Vector(0, 1),
			Vector(0, 0),
			Vector(1, 0),
			Vector(1, 1),
		};

		Vector attributeTangents[] =
		{
			Vector(-1, 0, 0),
			Vector(-1, 0, 0),
			Vector(-1, 0, 0),
			Vector(-1, 0, 0),
		};

		Index indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		//Create mesh
		CVertexBuilder vertexBuilder;
		SVertexMesh meshData;
		vertexBuilder.begin((uint32)ARRAYSIZE(attributePositions), (uint32)ARRAYSIZE(indices));
		vertexBuilder.setAttributeStream("POSITION", (const ts::byte*)(attributePositions), sizeof(Vector), eAttribFloat3);
		vertexBuilder.setAttributeStream("NORMAL",   (const ts::byte*)(attributeNormals),	sizeof(Vector), eAttribFloat3);
		vertexBuilder.setAttributeStream("TEXCOORD", (const ts::byte*)(attributeTexcoords), sizeof(Vector), eAttribFloat2);
		vertexBuilder.setAttributeStream("TANGENT",  (const ts::byte*)(attributeTangents),	sizeof(Vector), eAttribFloat3);
		vertexBuilder.setIndexStream(indices);
		vertexBuilder.end(meshData);
		//*/
		
		MeshId id;
		SMeshInstance meshInst;
		gfx->getMeshManager().createMesh(meshData, id);
		gfx->getMeshManager().getMeshInstance(id, meshInst);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create texture resources
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//*
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

		/*
		CTexture2D tex;
		if (!gfx->getTextureManager().loadTexture2D("logo_D.png", tex))
		{
			tswarn("tex fail");
		}
		else
		{
			hTex = tex.getHandle();
		}

		CTexture2D texDisp;
		if (!gfx->getTextureManager().loadTexture2D("logo_H.png", texDisp))
		{
			tswarn("tex fail");
		}
		else
		{
			hTexDisp = texDisp.getHandle();
		}

		CTexture2D texNorm;
		if (!gfx->getTextureManager().loadTexture2D("logo_N.png", texNorm))
		{
			tswarn("tex fail");
		}
		else
		{
			hTexNorm = texNorm.getHandle();
		}
		
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

		//drawCmd.mode = EDrawMode::eDrawIndexedInstanced;
		drawCmd.mode = EDrawMode::eDrawIndexed;
		drawCmd.constantBuffers[0] = hConstants;
		drawCmd.vertexStart = 0;
		drawCmd.vertexTopology = eTopologyPatchList3;
		//drawCmd.vertexTopology = eTopologyTriangleList;
		
		drawCmd.shaderVertex =	program.hVertex;
		drawCmd.shaderPixel  =	program.hPixel;
		drawCmd.shaderHull   =	program.hHull;
		drawCmd.shaderDomain =	program.hDomain;
		
		//drawCmd.vertexBuffers[0] = hVertexBuffer;
		//drawCmd.vertexBuffers[1] = hTexcoordBuffer;
		//drawCmd.vertexBuffers[2] = hNormalBuffer;
		//drawCmd.indexBuffer = hIndexBuffer;

		//drawCmd.vertexBuffers[0] = hVertexBuffer;
		//drawCmd.vertexOffsets[0] = 0;
		//drawCmd.vertexStrides[0] = meshData.vertexStride;
		//drawCmd.indexBuffer = hIndexBuffer;

		//drawCmd.vertexCount = vertexCount;
		//drawCmd.indexCount = indexCount;
		//drawCmd.vertexAttribCount = 4;

		drawCmd.vertexBuffers[0] = meshInst.vertexBuffers[0];
		drawCmd.vertexOffsets[0] = 0;
		drawCmd.vertexStrides[0] = meshInst.vertexStrides[0];
		drawCmd.indexBuffer = meshInst.indexBuffer;

		drawCmd.vertexCount = meshInst.vertexCount;
		drawCmd.indexCount = meshInst.indexCount;
		drawCmd.vertexAttribCount = meshInst.vertexAttributeCount;

		for (int i = 0; i < meshData.vertexAttributes.size(); i++)
		{
			drawCmd.vertexAttribs[i] = meshData.vertexAttributes[i];
		}

		//Texture
		drawCmd.textureUnits[0].arrayIndex = 0;
		drawCmd.textureUnits[0].arrayCount = 1;
		drawCmd.textureUnits[0].textureType = eTypeTexture2D;
		drawCmd.textureUnits[0].texture = hTex;
		//drawCmd.textureUnits[0].textureType = eTypeTextureCube;
		//drawCmd.textureUnits[0].texture = hCube;
		drawCmd.textureUnits[1].arrayIndex = 0;
		drawCmd.textureUnits[1].arrayCount = 1;
		drawCmd.textureUnits[1].textureType = eTypeTexture2D;
		drawCmd.textureUnits[1].texture = hTexDisp;
		drawCmd.textureUnits[2].arrayIndex = 0; 
		drawCmd.textureUnits[2].arrayCount = 1;
		drawCmd.textureUnits[2].textureType = eTypeTexture2D;
		drawCmd.textureUnits[2].texture = hTexNorm;
		//Texture sampler
		drawCmd.textureSamplers[0].addressU = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].addressV = ETextureAddressMode::eTextureAddressClamp;
		drawCmd.textureSamplers[0].addressW = ETextureAddressMode::eTextureAddressClamp;
		//drawCmd.textureSamplers[0].filtering = eTextureFilterTrilinear;
		drawCmd.textureSamplers[0].filtering = eTextureFilterPoint;
		drawCmd.textureSamplers[0].enabled = true;
		//Render states
		drawCmd.depthState.enableDepth = true;
		drawCmd.blendState.enable = false;
		drawCmd.rasterState.enableScissor = false;
		drawCmd.rasterState.cullMode = eCullNone;
		//drawCmd.rasterState.fillMode = eFillWireframe;
		drawCmd.rasterState.fillMode = eFillSolid;

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
		
		//Update shader constants
		Constants data;
		//data.world = Matrix::rotationY((float)timecount) * Matrix::translation(0.0f, 0.0f, 2.0f);
		data.world = Matrix::rotationY((Pi / 3) * sin((float)timecount)) * Matrix::translation(0.0f, 0.0f, 1.5f);
		data.view = Matrix::identity();
		data.projection = Matrix::perspectiveFieldOfView(Pi / 2, (float)config.width / config.height, 0.1f, 20.0f);
		
		data.lightDir = Vector(0, -0.5f, -1);
		data.lightDir = Matrix::transform3D(data.lightDir, data.view);
		data.lightDir.normalize();

		Matrix::transpose(data.world);
		Matrix::transpose(data.view);
		Matrix::transpose(data.projection);
		data.time = (float)timecount;
		data.tessScale = 0.05f;
		data.tessFactor = 50.0f;
		/*
		float interp = (float)sin(timecount);
		interp += 1.0f;
		interp /= 2.0f;
		data.tessFactor = ((20.0f - 1.0f) * interp) + 1.0f;
		*/

		//context->bufferUpdate(hMatrixBuffer, matrices);
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
		api->destroyTexture(hTexNorm);
		//api->destroyTexture(hCube);
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
