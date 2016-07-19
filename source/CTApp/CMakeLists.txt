
SET(CTApp_list
	config.cpp
	config.h
	exceptionhandler.cpp
	keycodes.h
	main.cpp
	window.cpp
	window.h
)

ADD_EXECUTABLE(
	CTApp
	${CTApp_list}
)

TARGET_LINK_LIBRARIES(CTApp CTCore)
TARGET_LINK_LIBRARIES(CTApp CTGraphics)