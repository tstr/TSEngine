#include <Windows.h>

#include "window.h"
#include "keycodes.h"
#include "config.h"

#include <C3E\core\console.h>
#include <C3E\core\utility.h>
#include <C3E\core\threading.h>
#include <C3E\core\assert.h>
#include <C3E\core\time.h>
#include <C3E\gfx\graphics.h>
#include <C3E\gfx\graphicsmodel.h>
#include <C3E\gfx\graphicstext.h>
#include <C3E\filesystem\filecore.h>
#include <C3E\filesystem\filexml.h>

#include <sstream>
#include <map>

/*
#include <C3E\Core.h>
#include <C3E\Core\typedefs.h>
#include <C3E\Core\threading.h>
#include <C3E\Core\LinearAlgebra.h>
#include <C3E\Graphics\ABI\core.h>
#include <C3E\Graphics\GfxCommon.h>
#include <C3E\Graphics\Gfx.h>
#include <C3E\Graphics\GfxModel.h>
#include <C3E\FileSystem\xml.h>
#include <C3E\FileSystem\xmlIterators.h>

C3Eapp.exe
C3E.dll
C3EGraphicsD3D11.dll
C3EGraphicsD3D12.dll
C3EGraphicsVK.dll
C3EGraphics.dll
C3ECore.dll

.cmat	//material
.c3d	//model
.cfx	//effect
.csc	//scene
*/

LINK_LIB("C3ECore.lib")
LINK_LIB("C3EGraphics.lib")

using namespace std;
using namespace C3E;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32 g_resX = 1280;
static uint32 g_resY = 720;

unique_ptr<Graphics> g_renderer;

atomic<bool> g_enabled = true;
atomic<bool> g_sim_active = true;
atomic<int> g_sim_count = 0;
mutex g_sim_mutex;

#define setbit(x) ( 1 << x )

enum InputMask
{
	Up =   setbit(0),
	Down = setbit(1),
	Left = setbit(2),
	Right= setbit(3),

	W = setbit(4),
	A = setbit(5),
	S = setbit(6),
	D = setbit(7),

	Q = setbit(8),
	Z = setbit(9),

	Shift = setbit(10)
};

atomic<float> g_fov = Pi / 2;

const float g_scrollDistanceInterval = 0.25f;
const float g_scrollDistanceMax = 10;
const float g_scrollDistanceMin = 0.7f;
atomic<float> g_inputWheel = 2.0f;
atomic<uint32> g_inputMask;

atomic<bool> g_mouseHeld = false;
atomic<int> g_mousePosX = 0;
atomic<int> g_mousePosY = 0;
atomic<int> g_mouseDeltaX;
atomic<int> g_mouseDeltaY;

atomic<bool> g_wireframe = false; //Toggle wireframe mode
atomic<bool> g_simulation = true; //Toggle light movement
atomic<bool> g_text = true;	      //Toggle text

atomic<bool> ui_clicked;

atomic<int> g_materialTextureMask;

//atomic<float> angleX = 0.0f;
//atomic<float> angleY = 0.0f;

int sim_lock()
{
	if (!g_sim_active) return 0;

	if (g_sim_count == 0)
		g_sim_mutex.lock();
	return ++g_sim_count;
}

int sim_unlock()
{
	if (!g_sim_active) return 0;

	int x = --g_sim_count;
	if (x == 0)
		g_sim_mutex.unlock();
	return x;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public Window
{
private:

	map<uint64, function<void(bool)>> m_keycallbacks;

public:

	MainWindow() : Window("C3EApp.exe") {}

	void OnCreate(WindowEventArgs e) override
	{
		ConsolePrintline("window created.");

		Window::OnCreate(e);
	}

	void OnDestroy(WindowEventArgs e) override
	{
		ConsolePrintline("window destroyed");
		sim_unlock();
		g_enabled = false;
		Window::OnDestroy(e);
	}

	void OnClose(WindowEventArgs e) override
	{
		ConsolePrintline("window closing...");

		if (MessageBoxA((HWND)id(), "Are you sure you want to exit?", "", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			Window::OnClose(e);
		}
		else
		{
			ConsolePrintline("window closing cancelled.");
		}
	}

	void OnSetfocus(WindowEventArgs e) override
	{
		ConsolePrintline("window focused: true");
		sim_unlock();
		Window::OnSetfocus(e);
	}

	void OnKillfocus(WindowEventArgs e) override
	{
		ConsolePrintline("window focused: false");
		sim_lock();
		Window::OnKillfocus(e);
	}

	void OnResize(WindowEventArgs e) override
	{
		Graphics::Viewport vp;
		vp.width = LOWORD(e.a);
		vp.height = HIWORD(e.a);

		sim_lock();
		g_resX = vp.width;
		g_resY = vp.height;
		g_renderer->SetViewport(vp);
		sim_unlock();

		//ConsolePrintline((string)"window resized: " + to_string(g_resX) + ", " + to_string(g_resY));
	}

	void OnScroll(WindowEventArgs e) override
	{
		float fraction = (float)GET_WHEEL_DELTA_WPARAM(e.b) / WHEEL_DELTA;

		float x = g_inputWheel.load();
		x += (fraction * g_scrollDistanceInterval);
		x = max(x, g_scrollDistanceMin);
		x = min(x, g_scrollDistanceMax);
		g_inputWheel.store(x);
		
		ConsolePrintline((string)"Camera zoom: " + to_string(x));
	}

	//Set a callback - true if keydown, false if keyup
	template<typename f_t> void SetKeyCallback(uint64 code, const f_t& f)
	{
		m_keycallbacks[code] = f;
	}
	
	void OnKeyup(WindowEventArgs e) override
	{
		auto it = m_keycallbacks.find(e.b);
		if (it != m_keycallbacks.end())
		{
			(*it).second(false);
		}

		if (e.b == VK_LEFT)
		{
			g_inputMask.fetch_and(~InputMask::Left);
		}
		else if (e.b == VK_RIGHT)
		{
			g_inputMask.fetch_and(~InputMask::Right);
		}
		else if (e.b == VK_DOWN)
		{
			g_inputMask.fetch_and(~InputMask::Down);
		}
		else if (e.b == VK_UP)
		{
			g_inputMask.fetch_and(~InputMask::Up);
		}
		else if (e.b == KEY_W)
		{
			g_inputMask.fetch_and(~InputMask::W);
		}
		else if (e.b == KEY_A)
		{
			g_inputMask.fetch_and(~InputMask::A);
		}
		else if (e.b == KEY_S)
		{
			g_inputMask.fetch_and(~InputMask::S);
		}
		else if (e.b == KEY_D)
		{
			g_inputMask.fetch_and(~InputMask::D);
		}
		else if (e.b == KEY_Q)
		{
			g_inputMask.fetch_and(~InputMask::Q);
		}
		else if (e.b == KEY_Z)
		{
			g_inputMask.fetch_and(~InputMask::Z);
		}
		else if (e.b == KEY_SHIFT)
		{
			g_inputMask.fetch_and(~InputMask::Shift);
		}
	}

	void OnKeydown(WindowEventArgs e) override
	{
		auto it = m_keycallbacks.find(e.b);
		if (it != m_keycallbacks.end())
		{
			(*it).second(true);
		}

		if (e.b == VK_LEFT)
		{
			if (g_inputMask & InputMask::Left) return;
			g_inputMask |= InputMask::Left;
		}
		else if (e.b == VK_RIGHT)
		{
			if (g_inputMask & InputMask::Right) return;
			g_inputMask |= InputMask::Right;
		}
		else if (e.b == VK_DOWN)
		{
			if (g_inputMask & InputMask::Down) return;
			g_inputMask |= InputMask::Down;
		}
		else if (e.b == VK_UP)
		{
			if (g_inputMask & (uint32)InputMask::Up) return;
			g_inputMask |= (uint32)InputMask::Up;
		}
		else if (e.b == KEY_W)
		{
			if (g_inputMask & InputMask::W) return;
			g_inputMask |= InputMask::W;
		}
		else if (e.b == KEY_A)
		{
			if (g_inputMask & InputMask::A) return;
			g_inputMask |= InputMask::A;
		}
		else if (e.b == KEY_S)
		{
			if (g_inputMask & InputMask::S) return;
			g_inputMask |= InputMask::S;
		}
		else if (e.b == KEY_D)
		{
			if (g_inputMask & InputMask::D) return;
			g_inputMask |= InputMask::D;
		}
		else if (e.b == KEY_Q)
		{
			if (g_inputMask & InputMask::Q) return;
			g_inputMask |= InputMask::Q;
		}
		else if (e.b == KEY_Z)
		{
			if (g_inputMask & InputMask::Z) return;
			g_inputMask |= InputMask::Z;
		}
		else if (e.b == KEY_SHIFT)
		{
			if (g_inputMask & InputMask::Shift) return;
			g_inputMask |= InputMask::Shift;
		}
	}

	void OnMouseDown(WindowEventArgs e) override
	{
		//cout << "Mouse pressed: x = " << LOWORD(e.a) << ", y = " << HIWORD(e.a) << endl;
		g_mouseHeld = true;
		ui_clicked = true;

		Window::OnMouseDown(e);
	}

	void OnMouseMove(WindowEventArgs e) override
	{
		//int xPos = LOWORD(e.a);
		//int yPos = HIWORD(e.a);
		g_mousePosX = LOWORD(e.a);
		g_mousePosY = HIWORD(e.a);

		Window::OnMouseMove(e);
	}

	void OnMouseUp(WindowEventArgs e) override
	{
		//cout << "Mouse released: x = " << LOWORD(e.a) << ", y = " << HIWORD(e.a) << endl;
		g_mouseHeld = false;
		Window::OnMouseUp(e);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Frustum
{
private:

	Plane m_planes[6];

public:

	Vector GetPlaneVector(int x) const
	{
		return (const Vector&)m_planes[x];
	}

	//View-projection matrix
	Frustum(const Matrix& matrix)
	{
		//*
		m_planes[0].x = matrix._14 + matrix._13;
		m_planes[0].y = matrix._24 + matrix._23;
		m_planes[0].z = matrix._34 + matrix._33;
		m_planes[0].w = matrix._44 + matrix._43;
		m_planes[0].Normalize();

		// Calculate far plane of frustum.
		m_planes[1].x = matrix._14 - matrix._13;
		m_planes[1].y = matrix._24 - matrix._23;
		m_planes[1].z = matrix._34 - matrix._33;
		m_planes[1].w = matrix._44 - matrix._43;
		m_planes[1].Normalize();

		// Calculate left plane of frustum.
		m_planes[2].x = matrix._14 + matrix._11;
		m_planes[2].y = matrix._24 + matrix._21;
		m_planes[2].z = matrix._34 + matrix._31;
		m_planes[2].w = matrix._44 + matrix._41;
		m_planes[2].Normalize();

		// Calculate right plane of frustum.
		m_planes[3].x = matrix._14 - matrix._11;
		m_planes[3].y = matrix._24 - matrix._21;
		m_planes[3].z = matrix._34 - matrix._31;
		m_planes[3].w = matrix._44 - matrix._41;
		m_planes[3].Normalize();

		// Calculate top plane of frustum.
		m_planes[4].x = matrix._14 - matrix._12;
		m_planes[4].y = matrix._24 - matrix._22;
		m_planes[4].z = matrix._34 - matrix._32;
		m_planes[4].w = matrix._44 - matrix._42;
		m_planes[4].Normalize();

		// Calculate bottom plane of frustum.
		m_planes[5].x = matrix._14 + matrix._12;
		m_planes[5].y = matrix._24 + matrix._22;
		m_planes[5].z = matrix._34 + matrix._32;
		m_planes[5].w = matrix._44 + matrix._42;
		m_planes[5].Normalize();
		//*/

		/*
		Matrix mat(matrix);

		mat[0];
		m_planes[0].x = mat[3] + mat[0];
		m_planes[0].y = mat[7] + mat[4];
		m_planes[0].z = mat[11] + mat[8];
		m_planes[0].w = mat[15] + mat[12];

		// Right Plane
		// col4 - col1
		m_planes[1].x = mat[3] - mat[0];
		m_planes[1].y = mat[7] - mat[4];
		m_planes[1].z = mat[11] - mat[8];
		m_planes[1].w = mat[15] - mat[12];

		// Bottom Plane
		// col4 + col2
		m_planes[2].x = mat[3] + mat[1];
		m_planes[2].y = mat[7] + mat[5];
		m_planes[2].z = mat[11] + mat[9];
		m_planes[2].w = mat[15] + mat[13];

		// Top Plane
		// col4 - col2
		m_planes[3].x = mat[3] - mat[1];
		m_planes[3].y = mat[7] - mat[5];
		m_planes[3].z = mat[11] - mat[9];
		m_planes[3].w = mat[15] - mat[13];

		// Near Plane
		// col4 + col3
		m_planes[4].x = mat[3] + mat[2];
		m_planes[4].y = mat[7] + mat[6];
		m_planes[4].z = mat[11] + mat[10];
		m_planes[4].w = mat[15] + mat[14];

		// Far Plane
		// col4 - col3
		m_planes[5].x = mat[3] - mat[2];
		m_planes[5].y = mat[7] - mat[6];
		m_planes[5].z = mat[11] - mat[10];
		m_planes[5].w = mat[15] - mat[14];

		*/
	}

	//Check if a point with optional radius lies within the view frustum.
	bool VECTOR_CALL CheckPoint(
		Vector pos,
		float radius = 0
	)
	{
		for (int i = 0; i < 6; i++)
		{
			if ((m_planes[i].DotCoordinate((Internal::XMVECTOR)pos)) < -radius)
			{
				return false;
			}
		}

		return true;
	}

	bool VECTOR_CALL CheckBox(
		const BoundingBox& box,
		Matrix mat = Matrix()
	)
	{
		/*
		Vector vmax(box.xMin, box.yMin, box.zMin);
		Vector vmin(box.xMax, box.yMax, box.zMax);
		Vector center = vmin + ((vmax - vmin) / 2.0f);
		float radius = (vmax - vmin).Length() / 2.0f;

		return CheckPoint(center, radius);


		//Check if box in frustum
		if (CheckPoint(Matrix::Transform(Vector(box.xMin, box.yMin, box.zMin), mat))) return true;
		if (CheckPoint(Matrix::Transform(Vector(box.xMax, box.yMax, box.zMax), mat))) return true;

		return false;

		if (CheckPoint(Matrix::Transform(Vector(box.xMax, box.yMin, box.zMin), mat))) return true;
		if (CheckPoint(Matrix::Transform(Vector(box.xMin, box.yMax, box.zMin), mat))) return true;
		if (CheckPoint(Matrix::Transform(Vector(box.xMax, box.yMax, box.zMin), mat))) return true;
		if (CheckPoint(Matrix::Transform(Vector(box.xMin, box.yMin, box.zMax), mat))) return true;
		if (CheckPoint(Matrix::Transform(Vector(box.xMax, box.yMin, box.zMax), mat))) return true;
		if (CheckPoint(Matrix::Transform(Vector(box.xMin, box.yMax, box.zMax), mat))) return true;


		*/

		/*
		//Check if frustum in box
		int out = 0;
		out = 0; for (int i = 0; i<8; i++) out += ((GetPlaneVector(i).x() > box.xMax) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i<8; i++) out += ((GetPlaneVector(i).x() < box.xMin) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i<8; i++) out += ((GetPlaneVector(i).y() > box.yMax) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i<8; i++) out += ((GetPlaneVector(i).y() < box.yMin) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i<8; i++) out += ((GetPlaneVector(i).z() > box.zMax) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i<8; i++) out += ((GetPlaneVector(i).z() < box.zMin) ? 1 : 0); if (out == 8) return false;
		//*/

		return false;

		/*
		Vector box[] =
		{
			Matrix::Transform(Vector(box.xMin, box.yMin, box.zMin), world),
			Matrix::Transform(Vector(box.xMax, box.yMax, box.zMax), world)
		};

		for (int i = 0; i < 6; i++)
		{
			const Plane& p = m_planes[i];

			//p-vertex selection
			const int px = static_cast<int>(p.x > 0.0f);
			const int py = static_cast<int>(p.y > 0.0f);
			const int pz = static_cast<int>(p.z > 0.0f);
			
			// Dot product
			// project p-vertex on plane normal
			// (How far is p-vertex from the origin)
			const float dp = p.Dot((DirectX::SimpleMath::Vector4)Vector(box[px].x(), box[py].y(), box[pz].z(), 0.0f));

			//http://www.txutxi.com/?p=584

			// Doesn't intersect if it is behind the plane
			if (dp < -p.w)
				return false;
		}

		return true;
		*/
	}
};

class BaseGeometry
{
protected:

	Graphics* m_renderer = nullptr;
	Matrix m_world;

	unique_ptr<GraphicsBuffer> m_vertexBuffer;
	unique_ptr<GraphicsBuffer> m_indexBuffer;

private:

	uint32 m_indexCount = 0;
	uint32 m_vertexCount = 0;

public:

	uint32 GetMaxIndexCount() const { return m_indexCount; }
	uint32 GetMaxVertexCount() const { return m_vertexCount; }

	const GraphicsBuffer* GetVertexBuffer() const { return m_vertexBuffer.get(); }
	const GraphicsBuffer* GetIndexBuffer() const { return m_indexBuffer.get(); }

	void VECTOR_CALL SetMatrix(Matrix mat) { m_world = mat; }

	BaseGeometry(Graphics* renderer) { m_renderer = renderer; }
	BaseGeometry(Graphics* renderer, const ModelImporter& loader);
	BaseGeometry(Graphics* renderer, const Vertex* vertices, uint32 vsize, const Index* indices = nullptr, uint32 isize = 0);
};

class Geometry : public BaseGeometry
{
private:

	struct SubMaterial
	{
		string m_name;
		shared_ptr<Effect> m_fx;
		shared_ptr<GraphicsBuffer> m_effectbuf;
		array<shared_ptr<Texture>, MATERIAL_TEXTURES_COUNT> m_texures;
	};

	struct SubMesh
	{
		SubMaterial* material = nullptr;
		uint32 startIndex = 0;
		uint32 vertexCount = 0;
		BoundingBox boundingbox;
	};

	vector<SubMesh> m_subMeshes;
	map<string, SubMaterial> m_subMaterials;

public:

	Geometry(Graphics* renderer, EffectFactory* factory, const ModelImporter& loader);
	~Geometry();

	Geometry(const Geometry&) = delete;

	const vector<SubMesh>& GetSubMeshArray() const { return m_subMeshes; }
};

class Sphere : public BaseGeometry
{
public:

	Sphere(Graphics* renderer, const ModelImporter& loader);
};

class Quad : public BaseGeometry
{
public:

	Quad(Graphics* renderer, int x, int y);
};

#define MATERIAL_TEX_DIFFUSE (1 << 0)
#define MATERIAL_TEX_NORMAL (1 << 1)
#define MATERIAL_TEX_SPECULAR (1 << 2)
#define MATERIAL_TEX_DISPLACE (1 << 3)

//atomic<uint32> g_materialTextureMask = UINT_MAX;

struct MaterialParams
{
	Vector diffuseColour;
	Vector ambientColour;
	Vector specularColour;
	float specularPower = 16;
	uint32 flags = 0;
	uint32 reserved[2];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GeometryBatch
{
private:

	vector<Vertex> m_vertices;
	unique_ptr<GraphicsBuffer> m_vertexbuffer;
	Graphics* m_renderer = nullptr;

public:

	GeometryBatch(Graphics* renderer) : m_renderer(renderer)
	{
		C3E_ASSERT(m_renderer);
	}

	void Begin()
	{
		m_vertices.clear();
	}

	void DrawQuad(float xpos, float ypos, float xscale, float yscale, Vector colour)
	{
		//float left = -1.0f * xscale + xpos;
		float left = 0 + xpos;
		float right = 1.0f * xscale + xpos;
		//float top = -1.0f * yscale + ypos;
		float top = 0 + ypos;
		float bottom = 1.0f * yscale + ypos;

		Vertex quad[6];

		quad[0].set((uint32)VertexAttributeIndex::Position, Vector(left, top, 0.0f, 1.0f));
		quad[0].set((uint32)VertexAttributeIndex::Colour, colour);

		quad[1].set((uint32)VertexAttributeIndex::Position, Vector(right, top, 0.0f, 1.0f));
		quad[1].set((uint32)VertexAttributeIndex::Colour, colour);

		quad[2].set((uint32)VertexAttributeIndex::Position, Vector(right, bottom, 0.0f, 1.0f));
		quad[2].set((uint32)VertexAttributeIndex::Colour, colour);

		quad[3].set((uint32)VertexAttributeIndex::Position, Vector(left, top, 0.0f, 1.0f));
		quad[3].set((uint32)VertexAttributeIndex::Colour, colour);

		quad[4].set((uint32)VertexAttributeIndex::Position, Vector(right, bottom, 0.0f, 1.0f));
		quad[4].set((uint32)VertexAttributeIndex::Colour, colour);

		quad[5].set((uint32)VertexAttributeIndex::Position, Vector(left, bottom, 0.0f, 1.0f));
		quad[5].set((uint32)VertexAttributeIndex::Colour, colour);

		//If not first vertex then insert a degenerate triangle
		if (m_vertices.size() > 0)
			m_vertices.push_back(quad[0]);

		for (size_t i = 0; i < 6; i++)
			m_vertices.push_back(quad[i]);

		m_vertices.push_back(quad[5]); //repeat last vertex
	}

	void End()
	{
		if (m_vertices.size() > 0)
		m_vertexbuffer.reset(new GraphicsBuffer(m_renderer, make_buffer_from_vector(m_vertices), EResourceType::TypeVertexBuffer));
	}

	~GeometryBatch() { }

	uint32 GetBuffers(GraphicsBuffer*& vb)
	{
		vb = m_vertexbuffer.get();
		return (uint32)m_vertices.size();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

shared_ptr<Effect> g_fxStandard;
shared_ptr<Effect> g_fxShadow;
shared_ptr<Effect> g_fxLightsource;
shared_ptr<Effect> g_fxSkybox;
shared_ptr<Effect> g_fxText;
shared_ptr<Effect> g_fxBasicColour;
shared_ptr<Effect> g_fxBasicTex;
shared_ptr<Effect> g_fxQuad;
shared_ptr<Effect> g_fxBlur;

int WINAPI WinMain(HMODULE Module, HMODULE prevModule, LPSTR cmdline, int showcmd)
{
	if (ConsoleCreateInstance())
		return EXIT_FAILURE;

	Frustum fr(Matrix::CreateLookTo(Vector(0, 0, 4), Vector(0, 0, 1), Vector(0, 1, 0)) * Matrix::CreatePerspectiveFieldOfView(Pi / 2, 16.0f / 9, 0.1f, 20.0f));
	bool b = fr.CheckPoint(Vector(20.0f, 0.0f, 10.0), 1.0f);
	
	//Loads program configuration from file
	Config config("config.xml");

	const char* val = nullptr;
	val = config.FindEntry("Settings\\Graphics\\AnisotropicFiltering", "0");
	auto AF = Graphics::AnisotropicFiltering(stoi(val));
	val = config.FindEntry("Settings\\Graphics\\Multisampling\\count", "1");
	int aacount = stoi(val);
	val = config.FindEntry("Settings\\Graphics\\Multisampling\\quality", "0");
	int aaquality = stoi(val);

	float AttenuationConstant = 1.0f;
	float AttenuationLinear = 0.16f;
	float AttenuationQuadratic = 0.0f;

	val = config.FindEntry("Settings\\Scene\\Light\\ConstantAttenuation", "1.0");
	AttenuationConstant = stof(val);
	val = config.FindEntry("Settings\\Scene\\Light\\LinearAttenuation", "0.16");
	AttenuationLinear = stof(val);
	val = config.FindEntry("Settings\\Scene\\Light\\QuadraticAttenuation", "0.0");
	AttenuationQuadratic = stof(val);
	
	val = config.FindEntry("Settings\\Graphics\\ResW", "1280");
	g_resX = (uint32)stoi(val);
	val = config.FindEntry("Settings\\Graphics\\ResH", "720");
	g_resY = (uint32)stoi(val);

	string directory_assets = config.FindEntry("Settings\\System\\AssetDirectory", "");
	string directory_fx_out = config.FindEntry("Settings\\System\\FX\\OutputDirectory", "");
	string directory_fx_in = config.FindEntry("Settings\\System\\FX\\WorkingDirectory", "");
	string file_model = config.FindEntry("Settings\\Scene\\Model", "");
	string file_skybox = config.FindEntry("Settings\\Scene\\Skybox", "");

	//Vector light_colour = { 1, 1, 1, 1 };
	//Vector ambient_colour = { 0.1f, 0.1f, 0.1f, 1 };

	Vector light_colour = { 1, 0.89f, 0.83f, 1 };
	Vector ambient_colour = { 0.26f, 0.24f, 0.23f, 1 };

	MainWindow win;

	ConsoleSetCommand("exit", [&](const char*){
		BringWindowToTop((HWND)win.id());
		win.Close();
	});

	ConsoleSetCommand("setfov", [&](const char* c)
	{
		if (c != "")
		{
			g_fov = stof(c) * (Pi / 180);
		}
	});

	ConsoleSetCommand("getfov", [&](const char*)
	{
		float d = g_fov.load() * (180.0f / Pi);
		ConsolePrintline(tocstr(d));
	});

	ConsoleSetCommand("setlightcolour", [&](const char* c)
	{
		stringstream ss(c);
		ss >> light_colour.x();
		ss >> light_colour.y();
		ss >> light_colour.z();
		ss >> light_colour.w();
	});

	ConsoleSetCommand("setambientcolour", [&](const char* c)
	{
		stringstream ss(c);
		ss >> ambient_colour.x();
		ss >> ambient_colour.y();
		ss >> ambient_colour.z();
		ss >> ambient_colour.w();
	});

	ConsoleSetCommand("resize", [&](const char* c)
	{
		string cmd(c);

		while (cmd.front() == ' ')
			cmd.erase(0, 1);

		size_t pos = cmd.find_first_of(' ');

		if (pos != string::npos && !cmd.empty())
		{
			uint32 x = stoi(cmd.substr(0, pos));
			uint32 y = stoi(cmd.substr(pos + 1, cmd.size() - 1));

			Graphics::Viewport view;
			g_renderer->GetViewport(view);
			
			/*
			if (win.IsFullscreen() || !view.fullscreen)
			{
				sim_lock();
				view.height = y;
				view.width = x;
				g_renderer->SetViewport(view);
				sim_unlock();

				printf("Resolution resized\n");
			}
			else
			*/
			{
				RECT dim;
				GetWindowRect((HWND)win.id(), &dim);

				win.Invoke([&](){
					BOOL result = SetWindowPos(
						(HWND)win.id(),
						HWND_TOP,
						((uint32_t)GetMonitorSizeX() - x) / 2,
						((uint32_t)GetMonitorSizeY() - y) / 2,
						x, y,
						SWP_SHOWWINDOW
					);
				});
			}
		}
	});

	ConsoleSetCommand("setconst", [&](const char* c){ AttenuationConstant = stof(c); });
	ConsoleSetCommand("setlinear", [&](const char* c){ AttenuationLinear = stof(c); });
	ConsoleSetCommand("setquadratic", [&](const char* c){ AttenuationQuadratic = stof(c); });

	ConsoleSetCommand("getconst", [&](const char* c){ cout << AttenuationConstant << endl; });
	ConsoleSetCommand("getlinear", [&](const char* c){ cout << AttenuationLinear << endl; });
	ConsoleSetCommand("getquadratic", [&](const char* c){ cout << AttenuationQuadratic << endl; });

	atomic<bool> text_scaling_up, text_scaling_down;
	atomic<bool> ui_messagebox_shown;
	atomic<bool> ui_enterkey_pressed;

	win.SetKeyCallback(KEY_T, [](bool iskeydown){ if (!iskeydown) g_text = !g_text; });
	win.SetKeyCallback(KEY_U, [](bool iskeydown){ if (!iskeydown) g_wireframe = !g_wireframe; });
	win.SetKeyCallback(KEY_Y, [](bool iskeydown){ if (!iskeydown) g_simulation = !g_simulation; });
	win.SetKeyCallback(VK_PRIOR, [&](bool iskeydown) { text_scaling_up = iskeydown; });
	win.SetKeyCallback(VK_NEXT, [&](bool iskeydown) { text_scaling_down = iskeydown; });
	win.SetKeyCallback(KEY_ESC, [&](bool iskeydown) { if (!iskeydown) { ui_messagebox_shown = !ui_messagebox_shown; }});
	win.SetKeyCallback(VK_RETURN, [&](bool iskeydown){ if (iskeydown) ui_enterkey_pressed = true; });

	/*
	win.SetKeyCallback(KEY_1, [&](bool ikd)
	{
		if (!ikd)
		{
			if (g_materialTextureMask & MATERIAL_TEX_NORMAL)
				g_materialTextureMask &= ~MATERIAL_TEX_NORMAL;
			else
				g_materialTextureMask |= MATERIAL_TEX_NORMAL;
		}
	});

	win.SetKeyCallback(KEY_2, [&](bool ikd)
	{
		if (!ikd)
		{
			if (g_materialTextureMask & MATERIAL_TEX_SPECULAR)
				g_materialTextureMask &= ~MATERIAL_TEX_SPECULAR;
			else
				g_materialTextureMask |= MATERIAL_TEX_SPECULAR;
		}
	});
	*/

	WindowRect winpos;

	winpos.h = g_resY;
	winpos.w = g_resX;
	winpos.x = ((uint32)GetMonitorSizeX() - g_resX) / 2;
	winpos.y = ((uint32)GetMonitorSizeY() - g_resY) / 2;

	//win.Create(winpos);
	win.AsyncCreate(winpos);

	char app_path[MAX_PATH];
	GetModuleFileNameA(0, app_path, MAX_PATH);
	win.SetTitle(app_path);

	Graphics::Configuration cfg;
	cfg.windowId = win.id();
	cfg.AAcount = aacount;
	cfg.AAquality = aaquality;
	cfg.anisotropicFiltering = AF;
	cfg.viewport.width = g_resX;
	cfg.viewport.height = g_resY;

#ifdef _DEBUG
	cfg.debug = true; //enable debug layer
#endif

	g_renderer.reset(new Graphics(Graphics::API_D3D11, cfg));

	/*

	const vector<Index> indices =
	{
		3, 1, 0,
		2, 1, 3,

		0, 5, 4,
		1, 5, 0,

		3, 4, 7,
		0, 4, 3,

		1, 6, 5,
		2, 6, 1,

		2, 7, 6,
		3, 7, 2,

		6, 4, 5,
		7, 4, 6
	};

	//Vertex buffer
	vector<Vertex> vertices(8);
	//top
	vertices[0].get((uint32)VertexAttributeIndex::Position) = Vector(-1.0f, 1.0f, -1.0f, 0.0f);
	vertices[1].get((uint32)VertexAttributeIndex::Position) = Vector(1.0f, 1.0f, -1.0f, 0.0f);
	vertices[2].get((uint32)VertexAttributeIndex::Position) = Vector(1.0f, 1.0f, 1.0f, 0.0f);
	vertices[3].get((uint32)VertexAttributeIndex::Position) = Vector(-1.0f, 1.0f, 1.0f, 0.0f);
	//bottom
	vertices[4].get((uint32)VertexAttributeIndex::Position) = Vector(-1.0f, -1.0f, -1.0f, 0.0f);
	vertices[5].get((uint32)VertexAttributeIndex::Position) = Vector(1.0f, -1.0f, -1.0f, 0.0f);
	vertices[6].get((uint32)VertexAttributeIndex::Position) = Vector(1.0f, -1.0f, 1.0f, 0.0f);
	vertices[7].get((uint32)VertexAttributeIndex::Position) = Vector(-1.0f, -1.0f, 1.0f, 0.0f);

	*/

	auto s = directory_assets + file_model;

	//system(((string)"E:\\programming\\c++\\C3E\\bin\\x64\\Debug\\ModelParser.exe " + s).c_str());

	ModelImporter meshloader(s.c_str());

	//Global scene parameters
	struct SceneParams
	{
		Matrix world;
		Matrix worldInv;
		Matrix view;
		Matrix viewInv;
		Matrix projection;
		Matrix projectionInv;

		Vector lightPosition;
		Vector lightColour;

		Vector globalAmbient;
		Vector eyePosition;

		float lightConstantAttenuation;
		float lightLinearAttenuation;
		float lightQuadraticAttenuation;

		float resolutionW = 0.0f;
		float resolutionH = 0.0f;

		float reserved[3];


		void InitMatrices()
		{
			projectionInv = projection.Inverse();
			worldInv = world.Inverse();
			viewInv = view.Inverse();
		}
	};

	SceneParams scene;
	SceneParams shadowScene;

	//Scene constant buffer
	GraphicsBuffer sceneBuffer(g_renderer.get(), make_buffer(scene), EResourceType::TypeShaderBuffer, EResourceUsage::UsageDynamic);
	GraphicsBuffer lightSceneBuffer(g_renderer.get(), make_buffer(scene), EResourceType::TypeShaderBuffer, EResourceUsage::UsageDynamic);
	GraphicsBuffer shadowSceneBuffer(g_renderer.get(), make_buffer(scene), EResourceType::TypeShaderBuffer, EResourceUsage::UsageDynamic);

	GraphicsQueue queue(g_renderer.get());

	//C3E_ASSERT(FileSystem::FileExists("effectdefs.cfxd"));
	//EffectFactory fxFactory(g_renderer.get());
	//C3E_ASSERT(fxFactory.CompileEffect("effectdefs.cfxd"));

	EffectFactory fxFactory(g_renderer.get());

	fxFactory.SetOutputDirectory(directory_fx_out.c_str());
	fxFactory.SetWorkingDirectory(directory_fx_in.c_str());

	g_fov = toRadians(stof(config.FindEntry("Settings\\Scene\\fov", "90")));

	if (stoi(config.FindEntry("Settings\\System\\FX\\Debug", "0")))
	{
		fxFactory.SetFlags(fxFactory.GetFlags() | EffectFactory::FX_DEBUG);
	}

	if (stoi(config.FindEntry("Settings\\System\\FX\\Recompile", "0")))
	{
		if (!fxFactory.CompileEffectManifest("manifest.fxm"))
		{
			char msg[] = "Effect factory could not compile effect definition file: ""manifest.fxm""";
			ConsolePrintline(msg);
			MessageBoxA(0, msg, "FX Compiler Error", MB_ICONERROR);

			BringWindowToTop((HWND)win.id());
			win.Close();
		}
	}

	uint32 shadowRes = max(1, stoi(config.FindEntry("Settings\\Graphics\\ShadowRes", "1024")));

	fxFactory.CreateEffect("BasicUnlitColourShader", g_fxBasicColour);
	fxFactory.CreateEffect("BasicUnlitTextureShader", g_fxBasicTex);
	fxFactory.CreateEffect("StandardShader", g_fxStandard);
	fxFactory.CreateEffect("LightSourceShader", g_fxLightsource);
	fxFactory.CreateEffect("ShadowmapShader", g_fxShadow);
	fxFactory.CreateEffect("QuadShader", g_fxQuad);
	fxFactory.CreateEffect("SkyboxShader", g_fxSkybox);
	fxFactory.CreateEffect("TextShader", g_fxText);
	fxFactory.CreateEffect("BlurShader", g_fxBlur);

	ModelImporter sphereloader((directory_assets + "sphere.c3d").c_str());

	const float scale = 0.02f;

	Geometry mesh(g_renderer.get(), &fxFactory, meshloader);
	Sphere lightSphere(g_renderer.get(), sphereloader);
	Quad quad(g_renderer.get(), 3, 3);

	/*
	DrawPacket packet;
	packet.SetEffect(&fx);
	packet.SetEffectBuffers(0, &sceneBuffer);
	packet.SetEffectBuffers(1, &materialBuffer);
	packet.SetEffectResources(0, &tex);
	packet.SetIndexBuffer(&indexBuffer);
	packet.SetVertexBuffers(0, &vertexBuffer);
	packet.SetVertexTopology(VertexTopology::TopologyTriangleList);
	packet.DrawIndexed(0, (uint32)indices.size());
	*/

	g_sim_active = true;
	double dt = 0.0;

	//angular velocity
	const double omega = Pi / 4;
	const double _speed = 2.5;
	double angleX = 0.0f;
	double angleY = 0.0f;

	Vector position(0,1,-1);
	Vector velocity;
	float pulsatance = 0.0f;

	//ShowWindow((HWND)win.id(), SW_MAXIMIZE);

	///////////////////////////////////////////////////////////////////////////////////////////

	int frame_count = 0;
	double frame_time_average = 0;
	const double frame_step = 0.2;

	//Deferred render target
	Multisampling ms;
	ms.count = aacount;
	ms.quality = aaquality;

	//GraphicsRenderTarget TextureRenderTarget(g_renderer.get(), g_resX, g_resY, ETextureFormat::FormatBGRA8_INT, ms);

	GraphicsRenderTarget shadowRenderTargets[6];
	
	for (int i = 0; i < 6; i++)
		shadowRenderTargets[i] = GraphicsRenderTarget(g_renderer.get(), shadowRes, shadowRes, ETextureFormat::FormatRG32_FLOAT);

	uint32 shadowScale = 1;

	GraphicsRenderTarget shadowRenderTargetCube(g_renderer.get(), shadowRes * shadowScale, shadowRes * shadowScale, ETextureFormat::FormatRG32_FLOAT, Multisampling(), true);

	TextureCube skybox(g_renderer.get(), (directory_assets + file_skybox).c_str());

	//Texture testtex(g_renderer.get(), ((string)directory_assets + "testimage.png").c_str());

	GeometryBatch geobatch(g_renderer.get());

	string caption;
	int curX = 0;
	int curY = 0;

	stringstream fontbuffer;
	fontbuffer.str("(Empty)");
	TextFactory font(g_renderer.get(), config.FindEntry("Settings\\System\\Font", "C:\\Windows\\Fonts\\verdana.ttf"), 40);

	Vector rayLightpos;

	///////////////////////////////////////////////////////////////////////////////////////////

	cout << "\n\n---------- Scene loaded ----------\n"
		<< "Use the arroykeys to rotate the camera\n"
		<< "Use WASD to move the camera\n"
		<< "Use QZ to move up and down\n"
		<< "Use U to toggle wireframe\n"
		<< "Use Y to toggle light movement\n"
		<< "Use T to toggle text\n"
		<< "Type ""list"" into the console to see the list of available console commands\n"
		<< "------------------------------------\n\n";


	/*
	HANDLE  hConsole;
	int k = 0;

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	// you can loop k higher to see more color choices
	for (k = 1; k < 255; k++)
	{
		// pick the colorattribute k you want
		SetConsoleTextAttribute(hConsole, k);
		cout << k << " I want to be nice today!" << endl;
	}
	*/

	///////////////////////////////////////////////////////////////////////////////////////////

	//reset input mask on entering program loop
	g_inputMask = 0;

	while (g_enabled)
	{
		lock_guard<mutex> lk(g_sim_mutex);

		Stopwatch t;
		t.Start();

		frame_count++;

		/*
		frame_time_average += dt;
		if (frame_time_average > frame_step)
		{
			stringstream buf;

			Graphics::DebugInfo inf;
			g_renderer->QueryDebugStatistics(inf);

			buf << "C3EApp.exe - Frametime: " << 1000 * frame_time_average / (double)frame_count << "ms"
				<< " - FPS: " << (double)frame_count / frame_time_average << "fps"
				<< " - DrawCalls: " << inf.drawcalls;

			win.SetTitle(buf.str().c_str());

			frame_time_average = 0.0;
			frame_count = 0;
		}
		*/

		Graphics::DebugInfo inf;
		g_renderer->QueryDebugStatistics(inf);

		g_renderer->DrawBegin(Colour(0.0f, 0.0f, 0.0f, 0));

		if (g_inputMask && !ui_messagebox_shown)
		{
			double speed = _speed;

			if (g_inputMask & InputMask::Shift)
			{
				speed *= 3;
			}

			if (g_inputMask & InputMask::Up)
				angleX -= omega * dt;
			if (g_inputMask & InputMask::Down)
				angleX += omega * dt;
			if (g_inputMask & InputMask::Left)
				angleY -= omega * dt;
			if (g_inputMask & InputMask::Right)
				angleY += omega * dt;

			if (g_inputMask & InputMask::W)
			{
				velocity.x() += (float)(speed * sin(angleY));
				velocity.z() += (float)(speed * cos(angleY));
			}
			if (g_inputMask & InputMask::S)
			{
				velocity.x() += (float)(-speed * sin(angleY));
				velocity.z() += (float)(-speed * cos(angleY));
			}
			if (g_inputMask & InputMask::D)
			{
				velocity.x() += (float)(speed * cos(angleY));
				velocity.z() += (float)(-speed * sin(angleY));
			}
			if (g_inputMask & InputMask::A)
			{
				velocity.x() += (float)(-speed * cos(angleY));
				velocity.z() += (float)(speed * sin(angleY));
			}
			if (g_inputMask & InputMask::Q)
			{
				velocity.y() += (float)speed;
			}
			if (g_inputMask & InputMask::Z)
			{
				velocity.y() -= (float)speed;
			}

			position += velocity * (float)dt;
			velocity = Vector();

			angleX = max(min(angleX, Pi / 2), -(Pi / 2));
			angleY = fmod(angleY, 2 * Pi);
			if (angleY <= 0) angleY += (Pi * 2);
		}

		if (g_simulation)
			pulsatance += (float)(((2.0 * Pi) / 5.0) * dt);
	
		//Displacement of light source
		Vector lightpos;
		lightpos.x() = 15.0f * sin(pulsatance = fmod(pulsatance, 2 * Pi));
		lightpos.y() = 4.0f + 2.0f * sin(pulsatance * 2.0f);
		lightpos.z() = 0.0f;

		lightpos.y() = 3.0f; //disable vertical displacement for now

		scene.world = Matrix();
		scene.view = Matrix();
		scene.projection = Matrix();

		Matrix view = (Matrix::CreateRotationX((float)angleX) * Matrix::CreateRotationY((float)angleY) * Matrix::CreateTranslation(position)).Inverse();
		Matrix projection = Matrix::CreatePerspectiveFieldOfView(g_fov, (float)g_resX / g_resY, 0.1f, 100.0f);

		caption = "";

		if (g_mouseHeld && !ui_messagebox_shown)
		{
			auto mouseX = (float)g_mousePosX.load();
			auto mouseY = (float)g_mousePosY.load();

			//mouseX = (float)g_resX / 2.0f;
			//mouseY = (float)g_resY / 2.0f;

			curX = (int)mouseX;
			curY = (int)mouseY;

			float x = (((2.0f * mouseX) / (float)g_resX) - 1);
			float y = -(((2.0f * mouseY) / (float)g_resY) - 1);

			//Ray start
			Vector vec0(x, y, 0.0f);
			vec0 = Matrix::Transform(vec0, projection.Inverse());
			//Ray end
			Vector vec1(x, y, 1.0f);
			vec1 = Matrix::Transform(vec1, projection.Inverse());

			//Ray direction
			auto viewInv = view.Inverse();
			//vec1.z() = 1.0f;
			//vec0.z() = 0.0f;
			//Vector ray = Matrix::Transform(vec1 - vec0, viewInv);
			float rayViewDepth = (vec1 - vec0).z();
			//cout << rayViewDepth << endl;

			Vector ray = Matrix::Transform(vec1, viewInv) - Matrix::Transform(vec0, viewInv);

			ray.Normalize();
			//ray.z() = 1.0f;
			//Ray origin
			Vector origin(position);

			//| l - c | =< r : check if picking ray intersects with light sphere
			if (((position + (ray * (scene.lightPosition - position).Length())) - scene.lightPosition).LengthSquared() <= (0.2f * 0.2f))
			{
				//update the light position based on ray cast
				rayLightpos = position + (ray * g_inputWheel);
			}
			else
			{
				cout << "Nothing selected\n";
			}

			Vector v = view.Forward();
			v.Normalize();
			//cout << toDegrees(acosf(Vector::Dot(ray, v))) << endl;

			lightpos = origin + (ray * g_inputWheel);

			/*
			const auto& submeshes = mesh.GetSubMeshArray();
			bool intersect = false;

			for (const auto& submesh : submeshes)
			{
				if (intersect)
				{
					break;
				}

				ModelBoundingBox box = submesh.boundingbox;
				Vector vmax(box.xMax, box.yMax, box.zMax);
				Vector vmin(box.xMin, box.yMin, box.zMin);
				
				DirectX::BoundingOrientedBox bb;
				Vector b[] = { vmin, vmax };
				DirectX::BoundingOrientedBox::CreateFromPoints(bb, 2, (const DirectX::XMFLOAT3*)b, sizeof(Vector));

				float f;
				intersect = bb.Intersects(origin, ray, f);
				//intersect = true;

				caption = submesh.material->m_name;

				if (intersect)
				{
					//cout << "Material: "<< submesh.material->m_name << endl;
				}
			}
			*/
		}

		//lightpos = rayLightpos;

		scene.view = view.Transpose();
		scene.projection = projection.Transpose();
		scene.eyePosition = view.Inverse().Translation();
		scene.eyePosition.w() = 1.0f;

		scene.resolutionW = (float)g_resX;
		scene.resolutionH = (float)g_resY;

		scene.lightColour = light_colour;
		scene.globalAmbient = ambient_colour;
		scene.lightPosition = lightpos;
		//scene.globalAmbient = Vector(0.2f, 0.2f, 0.2f, 1.0f);

		float lightFarPlane = 100.0f;
		float lightNearPlane = 0.1f;

		const Matrix lightProjection(Matrix::CreatePerspectiveFieldOfView(Pi / 2, 1.0f, lightNearPlane, lightFarPlane).Transpose());

		//View vectors for point light shadow cube
		Matrix lightViews[6];
		lightViews[0] = Matrix::CreateLookTo(scene.lightPosition, Vector(+1, 0, 0), Vector(0, +1, 0)).Transpose();
		lightViews[1] = Matrix::CreateLookTo(scene.lightPosition, Vector(-1, 0, 0), Vector(0, +1, 0)).Transpose();
		lightViews[2] = Matrix::CreateLookTo(scene.lightPosition, Vector(0, +1, 0), Vector(0, 0, -1)).Transpose();
		lightViews[3] = Matrix::CreateLookTo(scene.lightPosition, Vector(0, -1, 0), Vector(0, 0, +1)).Transpose();
		lightViews[4] = Matrix::CreateLookTo(scene.lightPosition, Vector(0, 0, +1), Vector(0, +1, 0)).Transpose();
		lightViews[5] = Matrix::CreateLookTo(scene.lightPosition, Vector(0, 0, -1), Vector(0, +1, 0)).Transpose();

		scene.lightConstantAttenuation = AttenuationConstant;
		scene.lightLinearAttenuation = AttenuationLinear;
		scene.lightQuadraticAttenuation = AttenuationQuadratic;

		//Computes inverse of world/view/projection
		scene.InitMatrices();

		//cout << Matrix::Transform(Vector(1, 1, 1, 1), scene.projection.Inverse()) << endl;
		//cout << scene.projection.Inverse() << endl;

		Frustum frustum(view * projection);

		/*
		cout << frustum.GetPlaneVector(0) << endl;
		cout << frustum.GetPlaneVector(1) << endl;
		cout << frustum.GetPlaneVector(2) << endl;
		cout << frustum.GetPlaneVector(3) << endl;
		cout << frustum.GetPlaneVector(4) << endl;
		cout << frustum.GetPlaneVector(5) << endl;
		cout << endl << endl;
		*/

		//*
		//Shadow pass
		if (frustum.CheckPoint(scene.lightPosition, lightFarPlane + lightNearPlane))
		{
			//1st pass
			for (int i = 0; i < 6; i++)
			{
				GraphicsPacket dp;

				shadowScene.eyePosition = scene.lightPosition;
				shadowScene.projection = lightProjection;
				shadowScene.view = lightViews[i];

				shadowScene.world = Matrix::CreateScale(scale, scale, scale);

				shadowScene.InitMatrices();

				Frustum shadowFrustum(shadowScene.view.Transpose() * shadowScene.projection.Transpose());

				dp.SetRenderTarget(&shadowRenderTargets[i]); //RTT
				dp.DisableDraw(false);
				dp.DisableDepth(false);
				dp.EnableBlending(false);

				dp.SetEffectBuffers(0, &shadowSceneBuffer);
				dp.UpdateBuffer(&shadowSceneBuffer, make_buffer(shadowScene));

				dp.SetVertexBuffers(0, mesh.GetVertexBuffer());
				dp.SetIndexBuffer(mesh.GetIndexBuffer());
				dp.SetEffect(g_fxShadow.get());
				dp.SetVertexTopology(VertexTopology::TopologyTriangleList);
				dp.DrawIndexed(0, mesh.GetMaxIndexCount());

				queue.Enqueue(dp);

				/*
				for (const auto& submesh : mesh.GetSubMeshArray())
				{
					//if (shadowFrustum.CheckBox(submesh.boundingbox))
					{
						dp.SetVertexBuffers(0, mesh.GetVertexBuffer());
						dp.SetIndexBuffer(mesh.GetIndexBuffer());
						dp.SetEffect(g_shadowFX.get());
						dp.SetVertexTopology(VertexTopology::TopologyTriangleList);
						dp.DrawIndexed(submesh.startIndex, submesh.vertexCount);

						queue.Enqueue(dp);
					}
				}
				//*/
			}

			//2nd pass
			for (int i = 0; i < 6; i++)
			{
				GraphicsPacket dp;

				dp.SetRenderTarget(&shadowRenderTargetCube, i);

				dp.UpdateBuffer(&sceneBuffer, make_buffer(scene));
				dp.SetEffectBuffers(0, &sceneBuffer);

				dp.DisableDraw(false);
				dp.DisableDepth(false);
				dp.EnableBlending(false);

				dp.SetEffect(g_fxBlur.get());
				dp.SetEffectResources(0, shadowRenderTargets[i].AsTexture());

				dp.SetVertexTopology(VertexTopology::TopologyTriangleList);
				dp.Draw(0, 6);

				queue.Enqueue(dp);
			}
		}

		//Skybox
		{
			GraphicsPacket dp;

			dp.SetEffectBuffers(0, &sceneBuffer);
			dp.UpdateBuffer(&sceneBuffer, make_buffer(scene));

			dp.SetEffect(g_fxSkybox.get());
			dp.SetEffectResources(0, &skybox);
			dp.SetVertexTopology(VertexTopology::TopologyTriangleList);
			dp.DisableDepth(true);
			dp.Draw(0, 6);

			queue.Enqueue(dp);
		}

		//*/

		//*
		//Colour pass
		{
			GraphicsPacket dp;

			dp.SetEffectBuffers(0, &sceneBuffer);

			scene.world = Matrix::CreateScale(scale, scale, scale);

			dp.UpdateBuffer(&sceneBuffer, make_buffer(scene));

			for (const auto& submesh : mesh.GetSubMeshArray())
			{
				//if (frustum.CheckBox(submesh.boundingbox))
				{
					//dp.SetRenderTarget(&TextureRenderTarget);

					dp.SetVertexBuffers(0, mesh.GetVertexBuffer());
					dp.SetIndexBuffer(mesh.GetIndexBuffer());
					dp.SetEffectBuffers(1, submesh.material->m_effectbuf.get());

					for (int i = 0; i < MATERIAL_TEXTURES_COUNT; i++)
					{
						if (submesh.material->m_texures[i].get())
						{
							dp.SetEffectResources(i, submesh.material->m_texures[i].get());
						}
					}

					dp.SetEffectResources(7, shadowRenderTargetCube.AsTexture());

					dp.SetEffect(submesh.material->m_fx.get());
					dp.SetVertexTopology(VertexTopology::TopologyTriangleList);

					if (g_wireframe)
					{
						dp.SetEffect(g_fxBasicTex.get());
						dp.SetVertexTopology(VertexTopology::TopologyLineList);
					}

					dp.DrawIndexed(submesh.startIndex, submesh.vertexCount);

					queue.Enqueue(dp);
				}
			}
		}

		//Draw light source
		if (frustum.CheckPoint(scene.lightPosition, 0.2f))
		{
			GraphicsPacket dp;

			Matrix m = Matrix::CreateTranslation(scene.lightPosition).Transpose();
			scene.world = m.Transpose();
			scene.worldInv = m.Inverse().Transpose();

			//dp.SetRenderTarget(&TextureRenderTarget);

			dp.SetEffectBuffers(0, &lightSceneBuffer);
			dp.UpdateBuffer(&lightSceneBuffer, make_buffer(scene));

			dp.SetVertexBuffers(0, lightSphere.GetVertexBuffer());
			dp.SetIndexBuffer(lightSphere.GetIndexBuffer());
			dp.SetEffect(g_fxLightsource.get());
			dp.SetVertexTopology(VertexTopology::TopologyTriangleList);
			dp.DrawIndexed(0, lightSphere.GetMaxIndexCount());

			queue.Enqueue(dp);
		}
		//*/

		/*
		//Textured quad
		{
			GraphicsPacket dp;

			dp.SetEffect(g_quadFX.get());

			dp.SetEffectBuffers(0, &sceneBuffer);
			//dp.SetVertexBuffers(0, quad.GetVertexBuffer());

			dp.SetEffectResources(0, font.GetTextureAtlas());
			//dp.SetEffectResources(0, TextureRenderTarget.AsTexture());
			//dp.SetEffectResources(1, TextureRenderTarget.AsDepthTexture());
			//dp.SetEffectResources(2, &cube);
			//dp.SetEffectResources(0, shadowRenderTarget.AsTexture());
			//dp.SetEffectResources(2, shadowRenderTarget.AsDepthTexture());

			dp.SetVertexTopology(VertexTopology::TopologyTriangleList);
			dp.Draw(0, 6);

			queue.Enqueue(dp);
		}
		//*/
	
		//*
		//Text pass
		if (g_text)
		{
			frame_time_average += dt;

			if (frame_time_average > frame_step)
			{
				fontbuffer.str("");

				fontbuffer << "Frametime: " << 1000 * frame_time_average / (double)frame_count << "ms\n"
					<< "FPS: " << (double)frame_count / frame_time_average << "fps\n"
					<< "Drawcalls: " << inf.drawcalls;

				frame_time_average = 0.0;
				frame_count = 0;
			}

			font.Begin(g_resX, g_resY);

			int32 x = 10;
			int32 y = 0;
			
			static float tscale = 0.6f;
			//tscale = 2 * abs(sin(pulsatance = fmod(pulsatance, 2 * Pi)));
			if (text_scaling_up) tscale += (float)(dt * 0.6);
			if (text_scaling_down) tscale -= (float)(dt * 0.6);

			Vector textcolour(scene.lightColour);
			textcolour = Vector( 1, 1, 1, 1 );

			font.DrawString(fontbuffer.str().c_str(), x, y, textcolour, tscale);

			if (caption != "")
			{
				font.DrawString(caption.c_str(), curX, curY, Vector(1, 0.8f, 0.8f, 1));
			}

			float resX = (float)g_resX;
			float resY = (float)g_resY;

			float originX = -resX / 2.0f;
			float originY = -resY / 2.0f;

			float sizeX = 20 * 25 * tscale;
			float sizeY = 20 * 8 * tscale;

			Vector uicolour = Vector(0.3f, 0.3f, 0.3f, 0.7f);

			geobatch.Begin();

			//UI - Debug panel
			//geobatch.DrawQuad(originX + 5.0f, (originY + resY) - (5.0f + sizeY), sizeX, sizeY, uicolour);

			//UI - Messagebox
			if (ui_messagebox_shown)
			{
				static uint32 selected = 0; //none
				const Vector unfocuscolour = Vector(0.4f, 0.4f, 0.4f, 0.8f);
				const Vector focuscolour = Vector(0.6f, 0.6f, 0.6f, 1);
				const Vector uitextcolour(1, 1, 1, 1);

				static Vector uibuttoncolour0 = unfocuscolour;
				static Vector uibuttoncolour1 = unfocuscolour;

				if (g_inputMask & InputMask::Left)
				{
					selected = 1;
					uibuttoncolour0 = focuscolour;
					uibuttoncolour1 = unfocuscolour;
				}
				else if (g_inputMask & InputMask::Right)
				{
					selected = 2;
					uibuttoncolour0 = unfocuscolour;
					uibuttoncolour1 = focuscolour;
				}

				float diagSizeX = 300.0f;
				float diagSizeY = 100.0f;

				float diagOriginX = originX + (resX / 2) - (diagSizeX / 2);
				float diagOriginY = (originY + (resY / 2)) - (diagSizeY / 2);

				float diagButtonSizeX = 100.0f;
				float diagButtonSizeY = 30.0f;

				float diagButton0offsetX = -60;
				float diagButton0offsetY = 0.0f;

				float diagButton1offsetX = 60;
				float diagButton1offsetY = 0.0f;

				//Message box background
				geobatch.DrawQuad(diagOriginX, diagOriginY, diagSizeX, diagSizeY, uicolour);


				float border = (float)GetSystemMetrics(SM_CYCAPTION) * 1;

				float mouseX = originX + (float)g_mousePosX;
				float mouseY = originY + (float)g_mousePosY;

				cout << mouseX << " " << mouseY << endl;
				cout << border << endl;
				
				{
					float x = diagOriginX + (diagSizeX / 2) - (diagButtonSizeX / 2) + diagButton0offsetX;
					float y = diagOriginY + (3.0f + diagButtonSizeY / 2 + diagButton0offsetY);

					int intersect = 0;
					if (mouseX > x) intersect++;
					if (mouseX < x + diagButtonSizeX) intersect++;
					if (mouseY > y + border) intersect++;
					if (mouseY < y + border + diagButtonSizeY) intersect++;

					if (intersect == 4)
					{
						selected = 1;
						uibuttoncolour0 = focuscolour;
						uibuttoncolour1 = unfocuscolour;
					}
					else
					{
						selected &= ~1;
					}

					//Message box button yes
					geobatch.DrawQuad(
						x,
						y,
						diagButtonSizeX,
						diagButtonSizeY,
						uibuttoncolour0
					);
				}

				{
					float x = diagOriginX + (diagSizeX / 2) - (diagButtonSizeX / 2) + diagButton1offsetX;
					float y = diagOriginY + (3.0f + diagButtonSizeY / 2 + diagButton1offsetY);

					int intersect = 0;
					if (mouseX > x) intersect++;
					if (mouseX < x + diagButtonSizeX) intersect++;
					if (mouseY > y + border) intersect++;
					if (mouseY < y + border + diagButtonSizeY) intersect++;

					if (intersect == 4)
					{
						selected = 2;
						uibuttoncolour0 = unfocuscolour;
						uibuttoncolour1 = focuscolour;
					}
					else
					{
						selected &= ~2;
					}

					//Message box button no
					geobatch.DrawQuad(
						x,
						y,
						diagButtonSizeX,
						diagButtonSizeY,
						uibuttoncolour1
					);
				}

				font.DrawString(
					"Are you sure you want to quit?",
					(int32)((resX / 2) - (diagSizeX / 2 - 20)),
					(int32)((resY / 2) - (diagSizeY / 2 - 10)),
					uitextcolour, 0.4f
				);

				font.DrawString(
					"Yes",
					(int32)((resX / 2) - (diagButtonSizeX / 2 - 35 - diagButton0offsetX)),
					(int32)((resY / 2) - (diagButtonSizeY / 2 - 20 - diagButton0offsetY)),
					uitextcolour, 0.35f
				);

				font.DrawString(
					"No",
					(int32)((resX / 2) - (diagButtonSizeX / 2 - 35 - diagButton1offsetX)),
					(int32)((resY / 2) - (diagButtonSizeY / 2 - 20 - diagButton1offsetY)),
					uitextcolour, 0.35f
				);

				if (ui_clicked)
				{
					if (selected == 1) //yes
					{
						win.Close();
					}
					if (selected == 2) //no
					{
						ui_messagebox_shown = false;
					}

					ui_enterkey_pressed = false;
					ui_clicked = false;
				}
			}

			geobatch.End();
			font.End();

			GraphicsPacket dp;

			//scene.projection = Internal::XMMatrixOrthographicLH(g_resX, g_resY, 0.1f, 20.0f);
			scene.projection = Matrix::CreateOrthographic((float)g_resX, (float)g_resY, 0, 1);
			scene.view = Matrix();
			scene.world = Matrix();

			dp.SetEffectBuffers(0, &sceneBuffer);
			dp.UpdateBuffer(&sceneBuffer, make_buffer(scene));

			//UI
			GraphicsBuffer* vbuffer = nullptr;
			uint32 numvertices = geobatch.GetBuffers(vbuffer);

			if (numvertices > 0)
			{
				dp.SetEffect(g_fxBasicColour.get());
				dp.DisableDepth(true);
				dp.EnableBlending(true);

				dp.SetVertexTopology(VertexTopology::TopologyTriangleStrip);

				dp.SetVertexBuffers(0, vbuffer);
				dp.SetIndexBuffer(nullptr);

				dp.Draw(0, numvertices);
				queue.Enqueue(dp);
			}

			//TEXT
			dp.SetEffect(g_fxText.get());
			dp.SetEffectResources(0, font.GetTextureAtlas());
			dp.DisableDepth(true);
			dp.EnableBlending(true);

			dp.SetVertexTopology(VertexTopology::TopologyTriangleStrip);
			//dp.SetVertexTopology((g_wireframe) ? VertexTopology::TopologyLineStrip : VertexTopology::TopologyTriangleStrip);

			GraphicsBuffer* vb = nullptr;
			GraphicsBuffer* ib = nullptr;
			uint32 numindices = font.GetBuffers(vb, ib);

			dp.SetVertexBuffers(0, vb);
			dp.SetIndexBuffer(ib);
			dp.DrawIndexed(0, numindices);

			queue.Enqueue(dp);
		}
		//*/

		//Dispatch command list to backend
		queue.Dispatch();

		g_renderer->DrawEnd();

		t.Stop();
		dt = t.DeltaTime() / 1000.0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////

	g_sim_active = false;

	ConsoleDestroyInstance();

	return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Geometry::Geometry(Graphics* renderer, EffectFactory* fxfactory, const ModelImporter& loader) :
	BaseGeometry(renderer, loader)
{
	for (uint32 x = 0; x < loader.GetMeshCount(); x++)
	{
		SubMesh smesh;
		//File mesh
		ModelMesh fmesh;
		loader.GetMesh(x, fmesh);

		///////////////////////////////////////////////////////////////////////////////////////////

		//Model material
		SubMaterial smat;
		//Shader material
		MaterialParams matParams;

		ModelMaterial fmat;
		if (!loader.GetMaterial(fmesh.nameMaterial, fmat))
		{
			cout << "Failed to load material '" << fmesh.nameMaterial << "'\n";
			continue;
		}

		matParams.ambientColour = fmat.cAmbient;
		matParams.diffuseColour = (fmat.cDiffuse != Vector()) ? fmat.cDiffuse : Vector(1,1,1,1);
		matParams.specularColour = fmat.cSpecular;
		//matParams.specularPower = 128.0f * (1.0f - fmat.shininess);
		matParams.specularPower = (fmat.shininess != 0) ? fmat.shininess : 120;

		string fx("StandardShader");

		/*
		if (fmat.effect == "")
		{
			cout << "Warning, material '" << fmat.name << "' does not have a valid effect\n";
			fmat.effect = "BasicEffect";
		}
		*/

		fxfactory->CreateEffect(fx.c_str(), smat.m_fx);

		smat.m_name = fmat.name;
		matParams.flags = 0;

		smat.m_texures[0] = shared_ptr<Texture>(new Texture(m_renderer, (loader.GetDirectory() + fmat.texDiffuse).c_str()));
		if (smat.m_texures[0]->good())
			matParams.flags |= MATERIAL_TEX_DIFFUSE;
		smat.m_texures[1] = shared_ptr<Texture>(new Texture(m_renderer, (loader.GetDirectory() + fmat.texNormal).c_str()));
		if (smat.m_texures[1]->good())
			matParams.flags |= MATERIAL_TEX_NORMAL;
		smat.m_texures[2] = shared_ptr<Texture>(new Texture(m_renderer, (loader.GetDirectory() + fmat.texDisplacement).c_str()));
		if (smat.m_texures[2]->good())
			matParams.flags |= MATERIAL_TEX_DISPLACE;
		smat.m_texures[3] = shared_ptr<Texture>(new Texture(m_renderer, (loader.GetDirectory() + fmat.texSpecular).c_str()));
		if (smat.m_texures[3]->good())
			matParams.flags |= MATERIAL_TEX_SPECULAR;

		//matParams.flags &= g_materialTextureMask;

		//Upload material buffer to gpu memory
		smat.m_effectbuf = unique_ptr<GraphicsBuffer>(new GraphicsBuffer(m_renderer, make_buffer(matParams), EResourceType::TypeShaderBuffer));

		/*
		for (int i = 0; i < MATERIAL_TEXTURES_COUNT; i++)
		{
			if (fmat.textures[i] != "")
			{
				smat.m_texures[i] = shared_ptr<Texture>(new Texture(m_renderer, (loader.GetDirectory() + fmat.textures[i]).c_str()));

				if (!smat.m_texures[i]->good())
				{
					cerr << "A texture was loaded incorrectly\n";
				}
			}
		}
		*/


		m_subMaterials[fmat.name] = smat;

		///////////////////////////////////////////////////////////////////////////////////////////

		smesh.material = &m_subMaterials[fmesh.nameMaterial];
		smesh.startIndex = fmesh.indexStart;
		smesh.vertexCount = fmesh.indexCount;

		smesh.boundingbox = fmesh.bounds;

		m_subMeshes.push_back(move(smesh));
	}
}

Geometry::~Geometry()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere(Graphics* renderer, const ModelImporter& loader) : BaseGeometry(renderer)
{
	vector<Vertex>vertices(loader.GetVertices(), loader.GetVertices() + loader.GetVertexCount());

	for (Vertex& v : vertices)
	{
		v.set((uint32)VertexAttributeIndex::Colour, Vector(1,1,1,1));
	}
	
	this->BaseGeometry::BaseGeometry(renderer, &vertices[0], (uint32)vertices.size(), loader.GetIndices(), loader.GetIndexCount());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Quad::Quad(Graphics* renderer, int x, int y) : BaseGeometry(renderer)
{
	/*
	float left = -1.0f;
	float right = 1.0f;
	float top = -1.0f;
	float bottom = 1.0f;

	Vertex quad[6];

	quad[0].set((uint32)VertexAttributeIndex::Position, Vector(left, top, 0.0f, 1.0f));
	quad[0].set((uint32)VertexAttributeIndex::Texcoord, Vector(0.0f, 0.0f));
	quad[0].set((uint32)VertexAttributeIndex::Colour, Vector(0.0f, 1.0f, 0.0f, 1.0f));

	quad[1].set((uint32)VertexAttributeIndex::Position, Vector(right, top, 0.0f, 1.0f));
	quad[1].set((uint32)VertexAttributeIndex::Texcoord, Vector(1.0f, 0.0f));
	quad[1].set((uint32)VertexAttributeIndex::Colour, Vector(0.0f, 1.0f, 0.0f, 1.0f));

	quad[2].set((uint32)VertexAttributeIndex::Position, Vector(right, bottom, 0.0f, 1.0f));
	quad[2].set((uint32)VertexAttributeIndex::Texcoord, Vector(1.0f, 1.0f));
	quad[2].set((uint32)VertexAttributeIndex::Colour, Vector(0.0f, 1.0f, 0.0f, 1.0f));

	quad[3].set((uint32)VertexAttributeIndex::Position, Vector(left, top, 0.0f, 1.0f));
	quad[3].set((uint32)VertexAttributeIndex::Texcoord, Vector(0.0f, 0.0f));
	quad[3].set((uint32)VertexAttributeIndex::Colour, Vector(0.0f, 1.0f, 0.0f, 1.0f));

	quad[4].set((uint32)VertexAttributeIndex::Position, Vector(right, bottom, 0.0f, 1.0f));
	quad[4].set((uint32)VertexAttributeIndex::Texcoord, Vector(1.0f, 1.0f));
	quad[4].set((uint32)VertexAttributeIndex::Colour, Vector(0.0f, 1.0f, 0.0f, 1.0f));

	quad[5].set((uint32)VertexAttributeIndex::Position, Vector(left, bottom, 0.0f, 1.0f));
	quad[5].set((uint32)VertexAttributeIndex::Texcoord, Vector(0.0f, 1.0f));
	quad[5].set((uint32)VertexAttributeIndex::Colour, Vector(0.0f, 1.0f, 0.0f, 1.0f));

	m_vertexBuffer.reset(new GraphicsBuffer(m_renderer, quad, sizeof(Vertex) * ARRAYSIZE(quad), EResourceType::TypeVertexBuffer));
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseGeometry::BaseGeometry(Graphics* renderer, const ModelImporter& loader) : m_renderer(renderer)
{
	m_vertexBuffer.reset(new GraphicsBuffer(m_renderer, MemoryBuffer(loader.GetVertices(), loader.GetVertexCount() * sizeof(Vertex)), EResourceType::TypeVertexBuffer));
	m_indexBuffer.reset(new GraphicsBuffer(m_renderer, MemoryBuffer(loader.GetIndices(), loader.GetIndexCount() * sizeof(Index)), EResourceType::TypeIndexBuffer));

	m_indexCount = loader.GetIndexCount();
	m_vertexCount = loader.GetVertexCount();
}

BaseGeometry::BaseGeometry(Graphics* renderer, const Vertex* vertices, uint32 vsize, const Index* indices, uint32 isize) : m_renderer(renderer)
{
	m_vertexBuffer.reset(new GraphicsBuffer(m_renderer, MemoryBuffer(vertices, vsize * sizeof(Vertex)), EResourceType::TypeVertexBuffer));
	if (indices) m_indexBuffer.reset(new GraphicsBuffer(m_renderer, MemoryBuffer(indices, isize * sizeof(Index)), EResourceType::TypeIndexBuffer));

	m_indexCount = isize;
	m_vertexCount = vsize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////