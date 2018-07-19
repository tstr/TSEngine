/*
	Graphics System header

	The graphics subsystem is responsible for controlling interactions between other systems and the low level render api
*/

#pragma once

#include <tsgraphics/abi.h>

#include <tscore/ptr.h>
#include <tscore/system/memory.h>
#include <tscore/path.h>
#include <tscore/signal.h>

#include "Driver.h"
#include "CommandQueue.h"
#include "Surface.h"
#include "RenderTargetPool.h"
#include "Image.h"
#include "Model.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	using DisplayEvent = Signal<DisplayConfig>;

	enum class DisplayMode
	{
		WINDOWED,
		BORDERLESS,
		FULLSCREEN
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
		uint multisampleLevel = 1;

		DisplayMode mode = DisplayMode::WINDOWED;
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

		//Graphics driver id
		RenderDriverID id = RenderDriverID::NONE;

		//Root asset loading path for textures/shaders/models
		Path rootpath;
	};

	/*
		Main Graphics Subsystem class
	*/
	class GraphicsSystem
	{
	private:
		
		struct System;
		OpaquePtr<System> pSystem;

		RenderDevice::Ptr pDevice;
		
	public:

		OPAQUE_PTR(GraphicsSystem, pSystem)

		//Initialize/deinitialize system
		TSGRAPHICS_API GraphicsSystem(const GraphicsConfig&);
		TSGRAPHICS_API ~GraphicsSystem();

		RenderDevice* device() const { return pDevice.get(); }

		/*
			Get/set graphics system properties
		*/

		TSGRAPHICS_API void refreshDisplay();

		TSGRAPHICS_API bool setDisplayResolution(uint w, uint h);
		TSGRAPHICS_API bool setDisplayMultisamplingLevel(uint ms);
		TSGRAPHICS_API bool setDisplayMode(DisplayMode mode);

		TSGRAPHICS_API void getDisplayOptions(GraphicsDisplayOptions& opt);

		TSGRAPHICS_API Path getRootPath() const;

		TSGRAPHICS_API ImageView getDisplayView() const;

		TSGRAPHICS_API ImageTargetPool* getDisplayTargetPool() const;
		Viewport getDisplayViewport() const { return getDisplayTargetPool()->getViewport(); }

		/*
			Load resources
		*/
		TSGRAPHICS_API const Image& getImage(const Path& path);

		TSGRAPHICS_API const Model& getModel(const Path& path);

		/*
			Pipeline methods
		*/

		//Signal draw begin
		TSGRAPHICS_API void begin();
		//Execute a queue of render commands
		TSGRAPHICS_API void execute(CommandQueue* queue);
		//Signal draw end
		TSGRAPHICS_API void end();

		/*
			Events
		*/

		DisplayEvent onDisplayChange;
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////