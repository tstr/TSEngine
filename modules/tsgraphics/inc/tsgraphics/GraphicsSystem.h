/*
	Graphics System header

	The graphics subsystem is responsible for controlling interactions between other systems and the low level render api
*/

#pragma once

#include <tsgraphics/abi.h>

#include <tscore/ptr.h>
#include <tscore/system/memory.h>
#include <tscore/path.h>

#include "GraphicsCore.h"
#include "CommandQueue.h"
#include "Surface.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum EDisplayMode
	{
		eDisplayWindowed,
		eDisplayBorderless,
		eDisplayFullscreen
	};

	/*
		Display configuration structure
	*/
	struct GraphicsDisplayOptions
	{
		//Display dimensions
		uint width = 0;
		uint height = 0;

		//Multisample count
		uint multisampleLevel;

		EDisplayMode mode = eDisplayWindowed;
	};

	/*
		Graphics System configuration structure
	*/
	struct GraphicsConfig
	{
		//Handle to drawing surface
		ISurface* surface = nullptr;

		//Display settings
		GraphicsDisplayOptions display;

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

		//Initialize/deinitialize system
		TSGRAPHICS_API GraphicsSystem(const GraphicsConfig&);
		TSGRAPHICS_API ~GraphicsSystem();

		//Get list of available adapters
		TSGRAPHICS_API void getAdapterList(std::vector<SRenderAdapterDesc>& adapters);

		/*
			Get/set graphics system properties
		*/

		TSGRAPHICS_API void refreshDisplay();

		TSGRAPHICS_API bool setDisplayResolution(uint w, uint h);
		TSGRAPHICS_API bool setDisplayMultisamplingLevel(uint ms);
		TSGRAPHICS_API bool setDisplayMode(EDisplayMode mode);

		TSGRAPHICS_API void getDisplayOptions(GraphicsDisplayOptions& opt);

		TSGRAPHICS_API HTarget getDisplayTarget() const;
		TSGRAPHICS_API Path getRootPath() const;

		/*
			Pipeline methods
		*/

		//Signal draw begin
		TSGRAPHICS_API void begin();
		//Execute a queue of render commands
		TSGRAPHICS_API void execute(CommandQueue* queue);
		//Signal draw end
		TSGRAPHICS_API void end();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////