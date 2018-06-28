#
#	Sandbox
#

SET(src_list
	
	src/main.cpp
	src/Sandbox.h
	src/Sandbox.cpp
	src/Camera.h
	src/Camera.cpp
	src/Entity.h
	src/TransformComponent.h
	src/RenderComponent.h
	
	src/util/IniReader.h
	src/util/IniReader.cpp

	src/3D/ForwardRender.h
	src/3D/ForwardRender.cpp
	src/3D/ForwardRenderConstants.h
)

ADD_SAMPLE(
	NAME Sandbox
	SOURCES ${src_list}
	CONFIG config.ini
)

ASSIGN_SOURCE_GROUPS(${src_list})

TARGET_LINK_LIBRARIES(Sandbox PUBLIC tsengine)
