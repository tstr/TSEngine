#
#	tsgraphics
#

set(tsgraphics_inc

	inc/tsgraphics/abi.h
	
	inc/tsgraphics/rendermodule.h
	inc/tsgraphics/texturemanager.h
	inc/tsgraphics/shadermanager.h
	
	inc/tsgraphics/api/renderapi.h
	inc/tsgraphics/api/rendercommon.h
	
	inc/tsgraphics/indexbuffer.h
	inc/tsgraphics/vertexbuffer.h
	inc/tsgraphics/uniformbuffer.h
	inc/tsgraphics/buffercommon.h
	inc/tsgraphics/colour.h

	inc/tsgraphics/model/modeldefs.h
)

SET(tsgraphics_src
	
	src/api/DX11/DX11base.h
	src/api/DX11/DX11render.h
	src/api/DX11/DX11render.cpp
	src/api/DX11/DX11adapter.cpp
	src/api/DX11/DX11helpers.h
	src/api/DX11/DX11helpers.cpp
	src/api/DX11/DX11context.cpp
	src/api/DX11/DX11shaderCompiler.cpp
	
	src/rendermodule.cpp
	src/rendermoduleloader.cpp
	src/texturemanager.cpp
	
	src/shader/shadermanager.cpp
	src/shader/preprocessor.h
	src/shader/preprocessor.cpp
	
	src/renderbuffer.cpp
	
	src/platform/nvoptimus.cpp
	src/platform/borderless.h
	src/platform/borderless.cpp
)

SET(tsgraphics_files ${tsgraphics_src} ${tsgraphics_inc})

add_library_tse(
	tsgraphics
	"${tsgraphics_files}"
)

target_include_directories(
	tsgraphics
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

assign_source_groups(${tsgraphics_files})

TARGET_LINK_LIBRARIES(tsgraphics PUBLIC tscore)
