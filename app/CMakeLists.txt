
SET(src_list
	winmain.cpp
	application.cpp
	application.h
)

ADD_EXECUTABLE(
	application
	${src_list}
)

TARGET_LINK_LIBRARIES(application tsengine)