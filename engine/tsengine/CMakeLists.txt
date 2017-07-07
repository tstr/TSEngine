#
#	tsengine
#

SET(tsengine_inc
	
	inc/tsengine/abi.h
	
	inc/tsengine/Env.h
	inc/tsengine/EnvInfo.h
	inc/tsengine/VarTable.h
	inc/tsengine/Surface.h

	inc/tsengine/KeyCodes.h
	inc/tsengine/Input.h
)

SET(tsengine_src
	
	src/input/InputEvent.cpp
	src/input/InputSystem.cpp
	src/input/keycodes.cpp
	
	src/platform/Console.h
	src/platform/Console.cpp
	src/platform/Window.cpp
	src/platform/Window.h

	src/CmdArgs.h
	src/CmdArgs.cpp
	src/IniReader.cpp
	src/IniReader.h
	
	src/Env.cpp
	src/EnvError.cpp
	src/EnvInfo.cpp
	src/EnvConfig.cpp
)

SET(tsengine_files ${tsengine_src} ${tsengine_inc})

add_engine_library(
	NAME tsengine
	SOURCES ${tsengine_src}
	HEADERS ${tsengine_inc}
	HEADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_link_libraries(tsengine PUBLIC tscore tsgraphics)

target_include_directories(
	tsengine
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

assign_source_groups(${tsengine_files})
