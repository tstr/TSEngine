/*
	Graphics System header

	The graphics subsystem is responsible for controlling interactions between other systems and the low level render api
*/

#pragma once

#include <tsgraphics/abi.h>

#include <tsconfig.h>
#include <tscore/ptr.h>
#include <tscore/system/memory.h>
#include <tscore/filesystem/path.h>

#include "GraphicsCore.h"

#include <tsgraphics/ShaderManager.h>
#include <tsgraphics/TextureManager.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum EDisplayMode
	{
		eDisplayUnknown	   = 0,
		eDisplayWindowed   = 1,
		eDisplayBorderless = 2,
		eDisplayFullscreen = 3
	};

	class GraphicsContext;

	/*
		Display configuration structure
	*/
	struct SGraphicsDisplayInfo
	{
		//Display dimensions
		uint32 width = 0;
		uint32 height = 0;
		SMultisampling multisampling;
		EDisplayMode mode = eDisplayWindowed;
	};

	/*
		Graphics System configuration structure
	*/
	struct SGraphicsSystemInfo
	{
		//Handle to display (application window)
		intptr windowHandle = 0;

		//Display info
		SGraphicsDisplayInfo display;

		//Graphics API id
		EGraphicsAPIID apiid = EGraphicsAPIID::eGraphicsAPI_Null;

		//Root asset loading path for textures/shaders/models
		Path rootpath;
	};
	
	/*
		Main Graphics Subsystem class
	*/
	class GraphicsSystem : public GraphicsCore
	{
	private:
		
		struct System;
		OpaquePtr<System> pSystem;

	public:

		OPAQUE_PTR(GraphicsSystem, pSystem)

		////////////////////////////////////////////////////////////////////////////////

		//Initialize/deinitialize system
		TSGRAPHICS_API GraphicsSystem(const SGraphicsSystemInfo&);
		TSGRAPHICS_API ~GraphicsSystem();

		//Get resource managers
		TSGRAPHICS_API CTextureManager* getTextureManager();
		TSGRAPHICS_API CShaderManager* getShaderManager();

		//Get list of available adapters
		TSGRAPHICS_API void getAdapterList(std::vector<SRenderAdapterDesc>& adapters);

		////////////////////////////////////////////////////////////////////////////////
		// Get/set graphics system properties
		////////////////////////////////////////////////////////////////////////////////

		void setDisplayInfo(EDisplayMode displaymode, uint32 width = 0, uint32 height = 0, SMultisampling sampling = SMultisampling(0))
		{
			SGraphicsDisplayInfo config;
			config.mode = displaymode;
			config.height = height;
			config.width = width;
			config.multisampling = sampling;
			this->setDisplayInfo(config);
		}

		TSGRAPHICS_API void setDisplayInfo(const SGraphicsDisplayInfo& info);
		TSGRAPHICS_API void getDisplayInfo(SGraphicsDisplayInfo& info);
		
		TSGRAPHICS_API intptr getDisplayHandle() const;
		TSGRAPHICS_API Path getRootPath() const;

		////////////////////////////////////////////////////////////////////////////////

		//Execute a graphical context on this frame
		TSGRAPHICS_API void execute(GraphicsContext* context);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////