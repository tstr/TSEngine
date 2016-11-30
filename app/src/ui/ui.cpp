/*
	ui module source
*/

#include "ui.h"
#include "imgui/imgui.h"

#include <tscore/debug/log.h>
#include <tscore/debug/assert.h>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////
// ctor
/////////////////////////////////////////////////////////////////////////////////////////////////

UISystem::UISystem(CInputModule* inputmodule, CRenderModule* rendermodule) :
	m_rendermodule(rendermodule),
	m_inputmodule(inputmodule)
{
	tsassert(m_rendermodule);
	tsassert(m_inputmodule);

	if (!m_inputmodule->addEventListener(this))
		tswarn("unable to register input event listener");

	//Load font
	ImGuiIO& io = ImGui::GetIO();
	io.ImeWindowHandle = (void*)m_inputmodule->getWindow()->handle();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\fonts\\verdana.ttf", 14.0f);
	//io.Fonts->AddFontDefault();

	//Imgui key map
	io.KeyMap[ImGuiKey_Tab] = eKeyTab;
	io.KeyMap[ImGuiKey_LeftArrow] = eKeyArrowLeft;
	io.KeyMap[ImGuiKey_RightArrow] = eKeyArrowRight;
	io.KeyMap[ImGuiKey_UpArrow] = eKeyArrowUp;
	io.KeyMap[ImGuiKey_DownArrow] = eKeyArrowDown;
	io.KeyMap[ImGuiKey_PageUp] = eKeyPageUp;
	io.KeyMap[ImGuiKey_PageDown] = eKeyPageDown;
	io.KeyMap[ImGuiKey_Home] = eKeyHome;
	io.KeyMap[ImGuiKey_End] = eKeyEnd;
	io.KeyMap[ImGuiKey_Delete] = eKeyDelete;
	io.KeyMap[ImGuiKey_Backspace] = eKeyBackspace;
	io.KeyMap[ImGuiKey_Enter] = eKeyEnter;
	io.KeyMap[ImGuiKey_Escape] = eKeyEsc;
	io.KeyMap[ImGuiKey_A] = eKeyA;
	io.KeyMap[ImGuiKey_C] = eKeyC;
	io.KeyMap[ImGuiKey_V] = eKeyV;
	io.KeyMap[ImGuiKey_X] = eKeyX;
	io.KeyMap[ImGuiKey_Y] = eKeyY;
	io.KeyMap[ImGuiKey_Z] = eKeyZ;

	//Create graphical resources
	init();
}

UISystem::~UISystem()
{
	ImGui::Shutdown();

	//Destroy graphical resources
	destroy();

	m_inputmodule->removeEventListener(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Update function
/////////////////////////////////////////////////////////////////////////////////////////////////

void UISystem::begin(IRenderContext* context, double dt)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = (float)dt;
	io.DisplaySize = ImVec2((float)m_width, (float)m_height);
	m_currentContext = context;

	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	ImGui::NewFrame();
}

void UISystem::end(ResourceProxy rendertarget)
{
	Viewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.w = m_width;
	viewport.h = m_height;

	ImGui::Render();
	draw(m_currentContext, rendertarget, viewport);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Input event handlers
/////////////////////////////////////////////////////////////////////////////////////////////////

int UISystem::onMouse(int16 dx, int16 dy)
{
	ImGuiIO& io = ImGui::GetIO();

	int16 x = 0;
	int16 y = 0;
	m_inputmodule->getCursorPos(x, y);
	io.MousePos.x = x;
	io.MousePos.y = y;

	return 0;
}

int UISystem::onMouseDown(const SInputMouseEvent& args)
{
	ImGuiIO& io = ImGui::GetIO();

	if (args.buttons == EMouseButtons::eMouseButtonLeft)
		io.MouseDown[0] = true;
	if (args.buttons == EMouseButtons::eMouseButtonRight)
		io.MouseDown[1] = true;
	if (args.buttons == EMouseButtons::eMouseButtonMiddle)
		io.MouseDown[2] = true;

	return 0;
}

int UISystem::onMouseUp(const SInputMouseEvent& args)
{
	ImGuiIO& io = ImGui::GetIO();

	if (args.buttons == EMouseButtons::eMouseButtonLeft)
		io.MouseDown[0] = false;
	if (args.buttons == EMouseButtons::eMouseButtonRight)
		io.MouseDown[1] = false;
	if (args.buttons == EMouseButtons::eMouseButtonMiddle)
		io.MouseDown[2] = false;

	return 0;
}

int UISystem::onMouseScroll(const SInputMouseEvent& args)
{
	ImGuiIO& io = ImGui::GetIO();

	io.MouseWheel += args.deltaScroll;
	return 0;
}

int UISystem::onKeyDown(EKeyCode code)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[code] = 1;
	//io.AddInputCharacter();

	return 0;
}

int UISystem::onKeyUp(EKeyCode code)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[code] = 0;

	return 0;
}

int UISystem::onChar(wchar ch)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(ch);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////