/*
	Render test
*/

#include <tsengine.h>

#include <tscore/debug/log.h>
#include <tscore/debug/profiling.h>

#include <tsgraphics/GraphicsContext.h>
#include <tsengine/input/inputmodule.h>

using namespace std;
using namespace ts;

//Use a cube mesh
#define USE_CUBE

////////////////////////////////////////////////////////////////////////////////////////////////

void generateCubeMesh(Vector halfextents, vector<Index>& indices, vector<Vector>& vertices, vector<Vector>& texcoords, vector<Vector>& normals);

////////////////////////////////////////////////////////////////////////////////////////////////

class Sample :
	public IApplication,
	public IInputEventListener,
	public GraphicsContext
{
private:

	CEngineEnv& mEnv;

	size_t frame = 0;
	double timecount = 0.0;

	HBuffer hConstants;
	HDrawCmd hDrawSolid;
	HDrawCmd hDrawWire;

	bool toggleSolid = true;

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

	//Create a mesh object
	MeshId loadMesh()
	{
		//Create mesh
		CVertexBuilder vertexBuilder;
		SVertexMesh meshData;

#ifdef USE_CUBE

		//Vertex buffer
		vector<Index> indices;
		vector<Vector> vertices;
		vector<Vector> texcoords;
		vector<Vector> normals;

		generateCubeMesh(Vector(0.5f, 0.5f, 0.5f), indices, vertices, texcoords, normals);

		vertexBuilder.begin((uint32)vertices.size(), (uint32)indices.size());
		vertexBuilder.setAttributeStream("POSITION", vertices, eAttribFloat3);
		vertexBuilder.setAttributeStream("NORMAL", normals, eAttribFloat3);
		vertexBuilder.setAttributeStream("TEXCOORD", texcoords, eAttribFloat2);
		vertexBuilder.setAttributeStream("TANGENT", normals, eAttribFloat3);
		vertexBuilder.setIndexStream(indices);
		vertexBuilder.end(meshData);

		meshData.vertexTopology = eTopologyTriangleList;

#else

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

		vertexBuilder.begin((uint32)ARRAYSIZE(attributePositions), (uint32)ARRAYSIZE(indices));
		vertexBuilder.setAttributeStream("POSITION", (const ts::byte*)(attributePositions), sizeof(Vector), eAttribFloat3);
		vertexBuilder.setAttributeStream("NORMAL", (const ts::byte*)(attributeNormals), sizeof(Vector), eAttribFloat3);
		vertexBuilder.setAttributeStream("TEXCOORD", (const ts::byte*)(attributeTexcoords), sizeof(Vector), eAttribFloat2);
		vertexBuilder.setAttributeStream("TANGENT", (const ts::byte*)(attributeTangents), sizeof(Vector), eAttribFloat3);
		vertexBuilder.setIndexStream(indices);
		vertexBuilder.end(meshData);
#endif

		MeshId id = 0;
		//Create mesh buffers
		getMeshManager()->createMesh(meshData, id);

		return id;
	}

public:
	
	Sample(CEngineEnv& env) :
		mEnv(env),
		GraphicsContext::GraphicsContext(env.getGraphics())
	{
		mEnv.getInput()->addEventListener(this);
	}

	//Input handler
	int onKeyDown(EKeyCode code) override
	{
		if (code == eKeyT)
		{
			toggleSolid = !toggleSolid;
		}

		return 0;
	}

	int onInit() override
	{
		GraphicsSystem* gfx = mEnv.getGraphics();
		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Initialize resources
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		ShaderId programId = 0;
		const string programName("TestCube");
		
		//Load shader program
		if (EShaderManagerStatus s = gfx->getShaderManager()->load(programName, programId))
		{
			tserror("Unable to load shader \"%\" : %", programName, s);
			return -1;
		}

		//Create shader constant buffer
		Constants data;
		hConstants = getPool()->createConstantBuffer(data);
		if (hConstants == HBUFFER_NULL)
			tswarn("buffer fail");

		//Create mesh
		MeshId id = loadMesh();
		SMeshInstance meshInst;
		getMeshManager()->getMeshInstance(id, meshInst);

		//Create textures
		TextureId tex = 0;
		TextureId texDisp = 0;
		TextureId texNorm = 0;

		STextureProperties texProps;
		
		if (auto status = gfx->getTextureManager()->load("cubetexture.png", tex, 0))
		{
			tswarn("tex fail (%)", status);
			return -1;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Create command
		CDrawBuilder drawBuild(mEnv.getGraphics());

		drawBuild.setShader(programId);
		drawBuild.setConstantBuffer(0, hConstants);

		drawBuild.setTexture(0, tex);
		drawBuild.setTexture(1, texDisp);
		drawBuild.setTexture(2, texNorm);
		
		drawBuild.setVertexBuffer(0, meshInst.vertexBuffers[0], meshInst.vertexStrides[0], meshInst.vertexOffset[0], meshInst.vertexAttributes, meshInst.vertexAttributeCount);
		drawBuild.setVertexTopology(eTopologyPatchList3);
		drawBuild.setIndexBuffer(meshInst.indexBuffer);

		STextureSampler sampler;
		sampler.addressU = ETextureAddressMode::eTextureAddressClamp;
		sampler.addressV = ETextureAddressMode::eTextureAddressClamp;
		sampler.addressW = ETextureAddressMode::eTextureAddressClamp;
		sampler.filtering = eTextureFilterAnisotropic16x;
		sampler.enabled = true;
		drawBuild.setTextureSampler(0, sampler);

		SDepthState depthState;
		SRasterState rasterState;
		SBlendState blendState;

		depthState.enableDepth = true;
		blendState.enable = false;
		rasterState.enableScissor = false;
		rasterState.cullMode = eCullNone;
		//rasterState.fillMode = eFillWireframe;
		rasterState.fillMode = eFillSolid;

		drawBuild.setRasterState(rasterState);
		drawBuild.setBlendState(blendState);
		drawBuild.setDepthState(depthState);

		drawBuild.setDrawIndexed(0, 0, meshInst.indexCount);

		//One command for solid drawing
		if (auto r = this->createDraw(drawBuild, hDrawSolid))
			tswarn("draw cmd fail : %", r);

		//Change fill mode
		rasterState.fillMode = eFillWireframe;
		drawBuild.setRasterState(rasterState);

		//Another command for wireframe drawing
		if (auto r = this->createDraw(drawBuild, hDrawWire))
			tswarn("draw cmd fail : %", r);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////

		return 0;
	}

	CommandQueue* renderFrame(HTarget target) override
	{
		auto gfx = mEnv.getGraphics();
		SGraphicsDisplayInfo displayInfo;
		gfx->getDisplayInfo(displayInfo);

		const Vector meshPos(0.0f, 0.0f, 3.0f);

		//Update shader constants
		Constants data;
		//data.world = Matrix::rotationY((float)timecount) * Matrix::translation(0.0f, 0.0f, 2.0f);
		data.world = Matrix::rotationY((Pi / 3) * sin((float)timecount)) * Matrix::translation(meshPos);
		data.view = Matrix::identity();
		data.projection = Matrix::perspectiveFieldOfView(Pi / 2, (float)displayInfo.width / displayInfo.height, 0.1f, 20.0f);

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

		CommandQueue* queue = this->getQueue();
		CommandBatch* batch = queue->createBatch();
		queue->addCommand(batch, CommandTargetClear(target, Vector((const float*)colours::AliceBlue), 1.0f));
		queue->addCommand(batch, CommandBufferUpdate(hConstants), data);
		queue->addCommand(batch, CommandDraw(target, (toggleSolid) ? hDrawSolid : hDrawWire, SViewport(displayInfo.width, displayInfo.height, 0, 0), SViewport()));
		queue->submitBatch(0, batch);

		queue->sort();

		return queue;
	}

	void onUpdate(double dt) override
	{
		timecount += dt;
		//tsprofile("% : %s", frame, timecount);
		frame++;

		this->update();
	}

	void onExit() override
	{

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
	Sample test(engine);
	return engine.start(test);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void generateCubeMesh(Vector halfextents, vector<Index>& indices, vector<Vector>& vertices, vector<Vector>& texcoords, vector<Vector>& normals)
{
	float x = halfextents.x();
	float y = halfextents.y();
	float z = halfextents.z();

	vertices = vector<Vector> {
		Vector(2, -2, 2),
		Vector(2, -2, -2),
		Vector(-2, -2, -2),
		Vector(-2, -2, 2),
		Vector(2, 2, 2),
		Vector(-2, 2, 2),
		Vector(-2, 2, -2),
		Vector(2, 2, -2),
		Vector(2, -2, 2),
		Vector(2, 2, 2),
		Vector(2, 2, -2),
		Vector(2, -2, -2),
		Vector(2, -2, -2),
		Vector(2, 2, -2),
		Vector(-2, 2, -2),
		Vector(-2, -2, -2),
		Vector(-2, -2, -2),
		Vector(-2, 2, -2),
		Vector(-2, 2, 2),
		Vector(-2, -2, 2),
		Vector(2, 2, 2),
		Vector(2, -2, 2),
		Vector(-2, -2, 2),
		Vector(-2, 2, 2),
	};

	//Scale vertices
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
		Vector(1, 1),
		Vector(1, 0),
		Vector(0, 0),
		Vector(0, 1),
		Vector(1, 1),
		Vector(1, 0),
		Vector(0, 0),
		Vector(0, 1),
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
