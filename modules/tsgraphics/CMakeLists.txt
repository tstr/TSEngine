#####################################################################################
#
#	tsgraphics
#
#	 Graphical subsystem module
#
#####################################################################################

set(tsgraphics_inc

	inc/tsgraphics/abi.h
	
	inc/tsgraphics/Surface.h
	inc/tsgraphics/GraphicsSystem.h
	inc/tsgraphics/GraphicsCore.h
	inc/tsgraphics/GraphicsContext.h
	inc/tsgraphics/TextureManager.h
	inc/tsgraphics/ShaderManager.h
	inc/tsgraphics/MeshManager.h
	inc/tsgraphics/BufferPool.h
	inc/tsgraphics/CommandQueue.h

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
	src/api/d3d11/StateManagerHash.inl
	
	src/api/d3d11/Adapter.cpp
)

SET(tsgraphics_src
	
	${tsgraphics_api_d3d11_src}
	
	src/GraphicsCore.cpp
	src/GraphicsSystem.cpp
	src/GraphicsContext.cpp

	src/CommandQueue.cpp
	src/CommandDispatchers.cpp
	
	src/ShaderManager.cpp
	src/TextureManager.cpp
	src/TextureImporter.h
	src/TextureImporter.cpp
	src/MeshManager.cpp
	src/VertexBuilder.cpp
	src/BufferPool.cpp
	src/GraphicsRenderPass.cpp
	
	src/platform/Nvoptimus.cpp
	
	src/model/Reader.h
	src/model/Reader.cpp
)

add_engine_module(
	NAME tsgraphics
	SOURCES ${tsgraphics_src}
	HEADERS ${tsgraphics_inc}
	PUBLIC_DIR inc
	PRIVATE_DIR src
)

TARGET_LINK_LIBRARIES(tsgraphics PUBLIC tscore)

#####################################################################################
#	Generate resource schema headers
#####################################################################################

include(ResourceSchemaTool)

set(schema_files
	inc/tsgraphics/schemas/Model.schema
	inc/tsgraphics/schemas/Shader.schema
)

target_cpp_rschemas(TARGET tsgraphics HEADER_VAR schemas_hdrs SCHEMAS ${schema_files} APPENDPATH)
assign_source_groups(${schemas_hdrs} ${schema_files})
INSTALL(FILES ${schemas_hdrs} ${schema_files} DESTINATION "${TS_HEADER_INSTALL}/tsgraphics/schemas")

#####################################################################################
