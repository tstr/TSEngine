/*
	Window helper functions for borderless mode
*/

#include "borderless.h"

#include <Windows.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	bool enterBorderless(intptr _hwnd)
	{
		HWND hwnd = (HWND)_hwnd;
		
		//Set borderless mode
		DEVMODE dev;
		ZeroMemory(&dev, sizeof(DEVMODE));

		int width = GetSystemMetrics(SM_CXSCREEN),
			height = GetSystemMetrics(SM_CYSCREEN);

		EnumDisplaySettings(NULL, 0, &dev);

		HDC context = GetWindowDC(hwnd);
		int colourBits = GetDeviceCaps(context, BITSPIXEL);
		int refreshRate = GetDeviceCaps(context, VREFRESH);

		dev.dmPelsWidth = width;
		dev.dmPelsHeight = height;
		dev.dmBitsPerPel = colourBits;
		dev.dmDisplayFrequency = refreshRate;
		dev.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		//todo: fix
		//LONG result = ChangeDisplaySettingsA(&dev, CDS_FULLSCREEN);// == DISP_CHANGE_SUCCESSFUL);
		//tserror("ChangeDisplaySettings returned %", result);
		
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, 0);
		BringWindowToTop(hwnd);
		
		return true;
	}
	
	bool exitBorderless(intptr _hwnd)
	{
		HWND hwnd = (HWND)_hwnd;

		//Exit borderless mode
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LEFT);
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		ShowWindow(hwnd, SW_RESTORE);
		
		return true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
