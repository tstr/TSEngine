
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

TARGET_LINK_LIBRARIES(application tsengine)