/*
	Shader compiler test.

	-	This application tests the ShaderC shader compiler tool

	-	Command line arguments:
		
		[0] Path to shaderc exe
		[1] Directory to load manifests from
		[2] Directory to write output files to

*/

#include <cstdio>
#include <cassert>
#include <iostream>
#include <thread>

#include <tsgraphics/GraphicsCore.h>
#include <tsgraphics/api/RenderApi.h>
#include <tsgraphics/ShaderManager.h>

#include <tscore/path.h>
#include <tscore/pathutil.h>

#include <Windows.h>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Graphics Tool System
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GraphicsToolSystem : public GraphicsCore
{
private:

	intptr m_handle = 0;
	Path m_loadpath;
	CShaderManager m_shaderManager;

public:

	GraphicsToolSystem(intptr winhandle, const Path& loadpath) :
		m_handle(winhandle),
		m_loadpath(loadpath)
	{}

	~GraphicsToolSystem()
	{
		this->deinit();
	}

	void init(EGraphicsAPIID id)
	{
		SRenderApiConfig config;
		config.adapterIndex = 0;
		config.display.fullscreen = false;
		config.display.multisampleLevel = 1;
		config.display.resolutionH = 1;
		config.display.resolutionW = 1;
		config.flags = eFlagDebug;
		config.windowHandle = m_handle;

		GraphicsCore::init(id, config);

		m_shaderManager = CShaderManager(this, m_loadpath, eShaderManagerFlag_Debug);
	}

	void deinit()
	{
		if (getApi())
		{
			m_shaderManager.clear();

			GraphicsCore::deinit();
		}
	}

	CShaderManager* getShaderManager() { return &m_shaderManager; }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Helper functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Print to console - todo: logging to file?
*/
void print(const string& msg)
{
	cout << msg << endl;
}

/*
	Launchs an executable with the given arguments
*/
int launch(const Path& path, const string& args)
{
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;

	StaticString<1024> buf(args.c_str());

	DWORD exitCode = EXIT_FAILURE;

	if (CreateProcessA(
		path.str(),
		buf.str(),
		nullptr,
		nullptr,
		true,
		0,
		nullptr,
		nullptr,
		&info, &processInfo
	))
	{
		WaitForSingleObject(processInfo.hProcess, INFINITE);

		GetExitCodeProcess(processInfo.hProcess, &exitCode);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}

	return exitCode;
}

static const char g_winClassName[] = "ProxyWindow-Shaderc";

/*
	Creates a dummy window
	GraphicsSystem class expects a HWND as an argument
*/
HWND createWindowProxy()
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(0);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = g_winClassName;

	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindowEx(0, g_winClassName, "dummy", 0, 0, 0, 1, 1, HWND_MESSAGE, NULL, NULL, NULL);

	if (!IsWindow(hwnd))
	{
		return nullptr;
	}

	thread thr([hwnd](){
	
		MSG msg;
		while (GetMessage(&msg, hwnd, NULL, NULL))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	});

	thr.detach();

	return hwnd;
}

/*
	Handles dummy window destruction/cleanup
*/
int destroyWindowProxy(HWND hwnd)
{
	DestroyWindow(hwnd);
	UnregisterClassA("ProxyWindow-Shaderc", GetModuleHandle(0));

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Entry point for test
*/
int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		print("ERROR: invalid arguments to test program");
		return EXIT_FAILURE;
	}

	string compilerPath(argv[1]);
	string inputPath(Path(argv[2]).str());
	string outputPath(Path(argv[3]).str());

	//Exit with failure if Shaderc tool does not exist
	if (!isFile(compilerPath))
	{
		print("ERROR: can't find shaderc.");
		return EXIT_FAILURE;
	}

	//Format command line arguments
	stringstream stream;
	stream << " --out ";
	stream << outputPath;
	stream << " ";
	stream << inputPath << "/shader.s";

	print(stream.str());
	
	//Invoke compiler - return process exit code if it fails
	if (int err = launch(compilerPath, stream.str()))
	{
		return err;
	}

	print("Creating proxy window...");

	//Create dummy window
	HWND hwnd = createWindowProxy();
	if (hwnd == nullptr)
	{
		print("ERROR: error creating proxy.");
		return EXIT_FAILURE;
	}

	print("Proxy window created.");

	IRender* api = nullptr;

	//Start system
	print("Initializing Graphics System...");
	GraphicsToolSystem system((intptr)hwnd, outputPath);
	system.init(eGraphicsAPI_D3D11);						//For now just test D3D11 configuration
	print("System initialized.");

	ShaderId programId = 0;
	
	print("Beginning shader verification...");

	//Try and load a shader
	if (EShaderManagerStatus status = system.getShaderManager()->load("shader", programId))
	{
		print("ERROR: Shader failed to verify (" + to_string(status) + ")");
		return (int)status;
	}

	print("Shaders verified successfully.");

	destroyWindowProxy(hwnd);

	return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
