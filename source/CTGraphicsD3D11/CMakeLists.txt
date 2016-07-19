
SET(CTGraphicsD3D11_list
	DX11Base.h
	DX11Main.cpp
	DX11RenderTarget.cpp
	DX11RenderTarget.h
	DX11ShaderCompiler.cpp
	DX11ShaderCompiler.h
	DX11Utils.h
	DXGIFormats.h
	pch.cpp
	pch.h
)

ADD_LIBRARY(
	CTGraphicsD3D11 SHARED
	${CTGraphicsD3D11_list}
)
