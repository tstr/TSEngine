#TSENGINE

SET(src_list
	
	../tsengine.h
	event/event.h
	event/event.cpp
	event/messenger.h
	
	input/inputdevice.h
	input/inputdevice.cpp
	input/keycodes.h
	input/keycodes.cpp
	input/inputmodule.h
	input/inputmodule.cpp
	
	platform/console.h
	platform/console.cpp
	platform/window.h
	platform/window.cpp

	tsengine.cpp
	cmdargs.cpp
	cmdargs.h
	configfile.h
	configfile.cpp
	cvar.h
	cvar.cpp
)

ADD_LIBRARY_EX(
	tsengine
	"${src_list}"
)


TARGET_LINK_LIBRARIES(tsengine tscore)
TARGET_LINK_LIBRARIES(tsengine tsgraphics)

copy_source_groups(${src_list})