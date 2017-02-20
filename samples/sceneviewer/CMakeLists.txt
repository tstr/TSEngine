#
#	3D Scene Viewers
#

SET(src_list
	
	src/main.cpp
	src/application.cpp
	src/application.h
	
	src/scene/modelimporter.h
	src/scene/modelimporter.cpp
	src/scene/camera.h
	src/scene/camera.cpp
	
	src/helpers/geometry.cpp
	src/helpers/geometry.h
	src/helpers/appinfo.h
	src/helpers/appinfo.cpp

	src/ui/ui.h
	src/ui/ui.cpp
	src/ui/ui_draw.cpp
	src/ui/imgui/imconfig.h
	src/ui/imgui/imgui.h
	src/ui/imgui/imgui.cpp
	src/ui/imgui/imgui_draw.cpp
	src/ui/imgui/imgui_internal.h

	src/ui/debugmenu.h
	src/ui/debugmenu.cpp
	src/ui/commandconsole.h
	src/ui/commandconsole.cpp
)

ADD_SAMPLE(
	sceneviewer
	"${src_list}"
)

ASSIGN_SOURCE_GROUPS(${src_list})

TARGET_LINK_LIBRARIES(sceneviewer PUBLIC tsengine)

#Copy config.ini file to the build folder
ADD_CUSTOM_COMMAND(
	TARGET sceneviewer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/config.ini" $<TARGET_FILE_DIR:sceneviewer>
	DEPENDS ${PROJECT_SOURCE_DIR}/config.ini
)
