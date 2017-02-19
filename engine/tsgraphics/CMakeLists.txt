#
#	tsgraphics
#

set(tsgraphics_inc

	inc/tsgraphics/abi.h
	
	inc/tsgraphics/GraphicsSystem.h
	inc/tsgraphics/TextureManager.h
	inc/tsgraphics/ShaderManager.h
	inc/tsgraphics/MeshManager.h
	
	inc/tsgraphics/api/RenderApi.h
	inc/tsgraphics/api/RenderDef.h

	inc/tsgraphics/Colour.h
	inc/tsgraphics/model/modeldefs.h
)

SET(tsgraphics_api_d3d11_src
	
	src/api/d3d11/Base.h
	src/api/d3d11/Helpers.h
	src/api/d3d11/Helpers.cpp
	
	src/api/d3d11/Render.h
	src/api/d3d11/RenderMain.cpp
	src/api/d3d11/RenderDisplay.cpp
	src/api/d3d11/RenderTexture.cpp
	src/api/d3d11/RenderShader.cpp
	src/api/d3d11/RenderTargets.cpp
	src/api/d3d11/RenderBuffers.cpp
	src/api/d3d11/RenderCommandDraw.cpp
	
	src/api/d3d11/Handle.h
	src/api/d3d11/HandleTexture.h
	src/api/d3d11/HandleShader.h
	src/api/d3d11/HandleTarget.h
	src/api/d3d11/HandleCommandDraw.h
	
	src/api/d3d11/Context.cpp
	src/api/d3d11/ContextCommandDraw.cpp
	src/api/d3d11/Context.h
	src/api/d3d11/StateManager.cpp
	src/api/d3d11/StateManager.h
	
	src/api/d3d11/Adapter.cpp
)

SET(tsgraphics_src
	
	${tsgraphics_api_d3d11_src}
	
	src/GraphicsSystem.cpp
	src/GraphicsSystemLoader.cpp
	src/TextureManager.cpp
	src/ShaderManager.cpp
	src/MeshManager.cpp
	src/VertexBuilder.cpp
	
	src/platform/Nvoptimus.cpp
	src/platform/Borderless.h
	src/platform/Borderless.cpp
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
