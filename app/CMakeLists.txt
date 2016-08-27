#Application

SET(src_list
	main.cpp
	application.cpp
	application.h
)

ADD_EXECUTABLE(
	application
	WIN32		#build exe as a win32 application
	${src_list}
)

copy_source_groups(${src_list})

TARGET_LINK_LIBRARIES(application tsengine)

#Copy config.ini file to the build folder
ADD_CUSTOM_COMMAND(
	TARGET application POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/config.ini" $<TARGET_FILE_DIR:application>
	DEPENDS ${PROJECT_SOURCE_DIR}/config.ini
)