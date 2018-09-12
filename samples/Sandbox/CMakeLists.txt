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

	src/2D/Painter.h
	src/2D/Painter.cpp

	src/3D/SceneRender.h
	src/3D/SceneRender.cpp
	src/3D/Renderable.h
	src/3D/Material.h
	src/3D/MaterialManager.h
	src/3D/MaterialManager.cpp
	src/3D/MaterialReader.h
	src/3D/MaterialReader.cpp
	src/3D/ShaderConstants.h
	src/3D/ShadowPass.h
	src/3D/ShadowPass.cpp
)

ADD_SAMPLE(
	NAME Sandbox
	SOURCES ${src_list}
	CONFIG config.ini
)

ASSIGN_SOURCE_GROUPS(${src_list})

TARGET_LINK_LIBRARIES(Sandbox PUBLIC tsengine)

ADD_DEPENDENCIES(Sandbox DATA_BUILD)
