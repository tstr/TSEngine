/*
	DXGI_FORMAT enum mapper
*/

#include <CT\core\corecommon.h>
#include "DX11Utils.h"

namespace CT
{
	template<typename t>
	struct dxgi_format_helper
	{
		static const DXGI_FORMAT value = DXGI_FORMAT_UNKNOWN;
	};

#define DECLARE_DXGI_F(x, y)\
	template<>\
	struct dxgi_format_helper<x>\
	{\
		static const DXGI_FORMAT value = y;\
	}\

	DECLARE_DXGI_F(int32, DXGI_FORMAT_R32_SINT);
	DECLARE_DXGI_F(uint32, DXGI_FORMAT_R32_UINT);
	DECLARE_DXGI_F(int16, DXGI_FORMAT_R16_SINT);
	DECLARE_DXGI_F(uint16, DXGI_FORMAT_R16_UINT);
}