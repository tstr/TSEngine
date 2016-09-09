#TSGRAPHICS

SET(src_list
	
	colour.h
	rendermodule.h
	rendermodule.cpp
	rendermoduleloader.cpp
	renderapi.h
	rendercommon.h
	
	texturemanager.h
	texturemanager.cpp
	shadermanager.h
	shadermanager.cpp
	
	indexbuffer.h
	vertexbuffer.h
	uniformbuffer.h
	renderbuffer.cpp
	buffercommon.h
	
	internal/nvoptimus.cpp
	model/modeldefs.h

	api/DX11/DX11base.h
	api/DX11/DX11render.h
	api/DX11/DX11render.cpp
	api/DX11/DX11adapter.cpp
	api/DX11/DX11helpers.h
	api/DX11/DX11helpers.cpp
	api/DX11/DX11context.cpp
	api/DX11/DX11shaderCompiler.cpp
)

ADD_LIBRARY_EX(
	tsgraphics
	"${src_list}"
)


TARGET_LINK_LIBRARIES(tsgraphics tscore)

copy_source_groups(${src_list})