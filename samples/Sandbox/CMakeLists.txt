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
	
	shaders/Common.h
	shaders/CommonLayouts.h
)

SET(shader_src_list
	shaders/Standard.fx
)

ADD_SAMPLE(
	NAME Sandbox
	SOURCES ${src_list}
	SHADERS ${shader_src_list}
	CONFIG config.ini
)

ASSIGN_SOURCE_GROUPS(${src_list} ${shader_src_list})

TARGET_LINK_LIBRARIES(Sandbox PUBLIC tsengine)
