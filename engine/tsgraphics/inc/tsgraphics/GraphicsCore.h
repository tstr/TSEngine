/*
	Graphics Core
	
	The base class for the graphics subsystem - controls the creation of the low level graphics interface and manages it's lifetime
*/

#pragma once

#include <tsgraphics/abi.h>

#include <tscore/ptr.h>
#include <tsgraphics/api/RenderDef.h>

namespace ts
{
	struct IRender;
	struct IRenderContext;

	enum EGraphicsAPIID
	{
		eGraphicsAPI_Null  = 0,
		eGraphicsAPI_D3D11 = 1
	};
	
	/*
		Graphics System base class
	*/
	class GraphicsCore
	{
	private:
		
		IRender* m_api = nullptr;
		EGraphicsAPIID m_apiid = eGraphicsAPI_Null;
		
	protected:
		
		TSGRAPHICS_API void init(EGraphicsAPIID, const SRenderApiConfig& config);
		TSGRAPHICS_API void deinit();
		
	public:
		
		IRender* const getApi() const { return m_api; }
		EGraphicsAPIID getApiID() const { return m_apiid; }
	};
}
