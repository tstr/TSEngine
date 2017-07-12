#
#	Sandbox
#

SET(src_list
	
	src/main.cpp
	src/Sandbox.h
	src/Sandbox.cpp
	src/SceneCamera.h
	src/SceneCamera.cpp
	src/Scene.cpp
	src/Scene.h
	src/Entity.h
	src/TransformComponent.h
	
	src/3D/Graphics3D.h
	src/3D/Graphics3D.cpp
	src/3D/Model.h
	src/3D/Model.cpp
	src/3D/util/IniReader.h
	src/3D/util/IniReader.cpp
	src/3D/MaterialInfo.h
	src/3D/ColourPass.h
	src/3D/ColourPass.cpp
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
