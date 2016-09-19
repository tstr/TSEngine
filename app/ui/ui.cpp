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
void f(...)
{

}
CUIModule::CUIModule(CInputModule* inputmodule, CRenderModule* rendermodule) :
	m_rendermodule(rendermodule),
	m_inputmodule(inputmodule)
{
	tsassert(m_rendermodule);
	tsassert(m_inputmodule);

	if (!m_inputmodule->addEventListener(this))
		tswarn("unable to register input event listener");

	ImGuiIO& io = ImGui::GetIO();
	io.ImeWindowHandle = (void*)m_inputmodule->getWindow()->handle();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\fonts\\verdana.ttf", 13.0f);
	//io.Fonts->AddFontDefault();

	//Create graphical resources
	init();
}

CUIModule::~CUIModule()
{
	//Destroy graphical resources
	destroy();
	ImGui::Shutdown();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Update function
/////////////////////////////////////////////////////////////////////////////////////////////////

void CUIModule::begin(IRenderContext* context, double dt)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = (float)dt;
	io.DisplaySize = ImVec2((float)m_width, (float)m_height);
	m_currentContext = context;

	ImGui::NewFrame();
}

void CUIModule::end(ResourceProxy rendertarget)
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

int CUIModule::onMouse(int16 dx, int16 dy)
{
	ImGuiIO& io = ImGui::GetIO();

	int16 x = 0;
	int16 y = 0;
	m_inputmodule->getCursorPos(x, y);
	io.MousePos.x = x;
	io.MousePos.y = y;

	return 0;
}

int CUIModule::onMouseDown(const SInputMouseEvent& args)
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

int CUIModule::onMouseUp(const SInputMouseEvent& args)
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

int CUIModule::onMouseScroll(const SInputMouseEvent& args)
{
	ImGuiIO& io = ImGui::GetIO();

	io.MouseWheel += args.deltaScroll;
	return 0;
}

int CUIModule::onKeyDown(EKeyCode code)
{
	return 0;
}

int CUIModule::onKeyUp(EKeyCode code)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////