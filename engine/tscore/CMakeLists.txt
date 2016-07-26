#TSCORE.lib

SET(src_list
	platform/window.h
	platform/window.cpp
)

ADD_LIBRARY(
	tscore STATIC
	${(src_list}
)
