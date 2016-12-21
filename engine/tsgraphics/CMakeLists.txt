#
#	tsgraphics
#

set(tsgraphics_inc

	inc/tsgraphics/abi.h
	
	inc/tsgraphics/graphicsSystem.h
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

SET(tsgraphics_api_d3d11_src
	
	src/api/d3d11/Base.h
	src/api/d3d11/Helpers.h
	src/api/d3d11/Helpers.cpp
	
	src/api/d3d11/Render.h
	src/api/d3d11/Render.cpp
	src/api/d3d11/RenderMain.cpp
	src/api/d3d11/RenderDisplay.cpp
	src/api/d3d11/RenderTexture.cpp
	src/api/d3d11/RenderShader.cpp
	src/api/d3d11/RenderTargets.cpp
	src/api/d3d11/RenderBuffers.cpp
	src/api/d3d11/Shader.h
	src/api/d3d11/Target.h
	src/api/d3d11/Context.cpp
	src/api/d3d11/Context.h
	src/api/d3d11/StateManager.cpp
	src/api/d3d11/StateManager.h
	
	src/api/d3d11/ShaderCompiler.cpp
	src/api/d3d11/Adapter.cpp
)

SET(tsgraphics_src
	
	${tsgraphics_api_d3d11_src}
	
	src/graphicsSystem.cpp
	src/graphicsSystemLoader.cpp
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
