#
#	Sandbox
#

SET(src_list
	
	src/main.cpp
	src/Sandbox.h
	src/Sandbox.cpp
	src/Camera.h
	src/Camera.cpp
	
	src/graphics/Graphics3D.h
	src/graphics/Graphics3D.cpp
	src/graphics/Model.h
	src/graphics/Model.cpp
	src/graphics/ConfigFile.h
	src/graphics/ConfigFile.cpp
)

SET(shader_src_list
	shaders/Common.h
	shaders/Standard.fx
)

SET(shader_manifest shaders/Sandbox.shm)

ADD_SAMPLE(
	NAME Sandbox
	SOURCES ${src_list}
	SHADERS ${shader_src_list}
	SHADER_MANIFEST ${shader_manifest}
	CONFIG config.ini
)

ASSIGN_SOURCE_GROUPS(${src_list} ${shader_src_list} ${shader_manifest})

TARGET_LINK_LIBRARIES(Sandbox PUBLIC tsengine)
