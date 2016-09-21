/*
	ui module header
*/

#pragma once

#include "../application.h"
#include "imgui/imgui.h"

namespace ts
{
	class CUIModule : public IInputEventListener
	{
	private:
		
		CInputModule* m_inputmodule = nullptr;
		CRenderModule* m_rendermodule = nullptr;

		//input handlers
		int onMouse(int16 dx, int16 dy) override;
		int onMouseDown(const SInputMouseEvent&)  override;
		int onMouseUp(const SInputMouseEvent&) override;
		int onMouseScroll(const SInputMouseEvent&) override;
		int onKeyDown(EKeyCode code) override;
		int onKeyUp(EKeyCode code) override;
		int onChar(wchar ch) override;

		//Graphics resources
		ResourceProxy m_textureAtlasRsc;
		ResourceProxy m_textureAtlasView;
		ResourceProxy m_textureAtlasSampler;

		CShader m_vertexShader;
		CShader m_pixelShader;
		ResourceProxy m_vertexInput;

		CUniformBuffer m_uniformBuffer;
		CVertexBuffer m_vertexBuffer;
		CIndexBuffer m_indexBuffer;

		uint32 m_vertexBufferSize = 0;
		uint32 m_indexBufferSize = 0;
		ImDrawIdx* m_cpuIndexBuffer = nullptr;
		ImDrawVert* m_cpuVertexBuffer = nullptr;

		uint32 m_height = 0;
		uint32 m_width = 0;
		IRenderContext* m_currentContext = nullptr;

		//internal methods
		void init();
		void draw(IRenderContext* context, ResourceProxy rendertarget, Viewport viewport);
		void destroy();

	public:
		
		CUIModule(CInputModule* inputmodule, CRenderModule* rendermodule);
		~CUIModule();

		CUIModule(const CUIModule&) = delete;

		void setDisplaySize(uint32 width, uint32 height)
		{
			m_height = height;
			m_width = width;
		}

		void begin(IRenderContext* context, double deltatime);
		void end(ResourceProxy rendertarget);
	};
}
