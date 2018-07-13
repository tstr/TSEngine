#####################################################################################
#
#	tsdx11
#
#	 D3D11 rendering backend for the graphics subsystem
#
#####################################################################################

SET(tsdx11_src
	
	src/Base.h
	src/Helpers.h
	src/Helpers.cpp
	
	src/Render.h
	src/RenderMain.cpp
	src/RenderDisplay.cpp
	
	src/Handle.h
	src/HandleResource.h
	src/HandleResource.cpp
	src/HandleResourceSet.h
	src/HandleResourceSet.cpp
	src/HandleTarget.h
	src/HandleTarget.cpp
	src/HandleShader.h
	src/HandleShader.cpp
	src/HandlePipeline.h
	src/HandlePipeline.cpp
	
	src/Context.h
	src/Context.cpp
	src/ContextCommand.cpp
	src/StateManager.h
	src/StateManager.cpp
	src/StateManagerHash.inl
)

SET(tsdx11_inc
    include/tsdx11.h
)

add_engine_module(
	NAME tsdx11
	SOURCES ${tsdx11_src}
	HEADERS ${tsdx11_inc}
	PUBLIC_DIR include
	PRIVATE_DIR src
)

TARGET_LINK_LIBRARIES(tsdx11 PUBLIC tscore)
TARGET_LINK_LIBRARIES(tsdx11 PUBLIC tsgraphics-interface)

#####################################################################################
