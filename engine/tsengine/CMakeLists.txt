#####################################################################################
#
#	tsengine
#
#	 Central system module
#
#####################################################################################

SET(tsengine_inc
	
	inc/tsengine/abi.h
	
	inc/tsengine/Env.h
	inc/tsengine/EnvInfo.h
	inc/tsengine/VarTable.h

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

add_engine_module(
	NAME tsengine
	SOURCES ${tsengine_src}
	HEADERS ${tsengine_inc}
	PUBLIC_DIR inc
	PRIVATE_DIR src
)

target_link_libraries(tsengine PUBLIC tscore tsgraphics)

#####################################################################################
