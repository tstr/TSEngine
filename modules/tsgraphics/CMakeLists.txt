#####################################################################################
#
#	tsgraphics
#
#	 Graphical subsystem module
#
#####################################################################################

set(tsgraphics_inc

	inc/tsgraphics/abi.h
	
	inc/tsgraphics/Colour.h
	inc/tsgraphics/Surface.h
	inc/tsgraphics/AssetCache.h
	inc/tsgraphics/CommandQueue.h
	inc/tsgraphics/FrameGraph.h
	inc/tsgraphics/BindingSet.h
    
	inc/tsgraphics/Defs.h
	inc/tsgraphics/Driver.h
	inc/tsgraphics/Graphics.h
    
	inc/tsgraphics/Buffer.h
	inc/tsgraphics/Shader.h
    inc/tsgraphics/Image.h
    inc/tsgraphics/Model.h
	inc/tsgraphics/RenderTarget.h
	inc/tsgraphics/RenderTargetPool.h
)

SET(tsgraphics_src

	src/Graphics.cpp
	src/Driver.cpp

	src/CommandQueue.cpp
	src/CommandDispatchers.cpp
	
    src/Shader.cpp
	src/Image.cpp
	src/Model.cpp
	src/RenderTarget.cpp
)

add_engine_module(
	NAME tsgraphics
	SOURCES ${tsgraphics_src}
	HEADERS ${tsgraphics_inc}
	PUBLIC_DIR inc
	PRIVATE_DIR src
)

TARGET_LINK_LIBRARIES(tsgraphics PUBLIC tscore)
TARGET_LINK_LIBRARIES(tsgraphics PRIVATE tsdx11)

#####################################################################################
#	Generate resource schema headers
#####################################################################################

include(ResourceSchemaTool)

set(schema_files
	inc/tsgraphics/schemas/Model.schema
	inc/tsgraphics/schemas/Shader.schema
    inc/tsgraphics/schemas/Image.schema
)

target_cpp_rschemas(TARGET tsgraphics HEADER_VAR schemas_hdrs SCHEMAS ${schema_files} APPENDPATH)
assign_source_groups(${schemas_hdrs} ${schema_files})
INSTALL(FILES ${schemas_hdrs} ${schema_files} DESTINATION "${TS_HEADER_INSTALL}/tsgraphics/schemas")

#####################################################################################
