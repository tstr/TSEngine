/*
	Surface Interface
*/

#pragma once

#include <tscore/types.h>

namespace ts
{
	/*
		Surface:
		
		A surface represents a rectangular drawing region, typically a window or control.
		
		This interface is used by the Graphics subsystem for rendering to.

		Surface's can be resized and can be in a "windowed" or "borderless" state
	*/
	class ISurface
	{
		//Put Surface into borderless mode, Surface is put into fullscreen and size is set to native resolution
		virtual void enableBorderless(bool enable) = 0;
		//Is Surface in borderless mode
		virtual bool isBorderless() const = 0;
		//Does platform support borderless mode
		virtual bool supportsBorderless() const = 0;
		
		//Redraw Surface
		virtual void redraw() = 0;
		//Resize Surface - will not work in borderless mode as size equals native resolution
		virtual void resize(uint width, uint height) = 0;
		//Get size of Surface
		virtual void getSize(uint& width, uint& height) const = 0;
		
		//Get platform specific Surface handle
		virtual intptr getHandle() const = 0;
	};
}
