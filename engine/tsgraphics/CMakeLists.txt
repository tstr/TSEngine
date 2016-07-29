#TSGRAPHICS

SET(src_list
	colour.h
)

ADD_LIBRARY(
	tsgraphics STATIC
	${src_list}
)


TARGET_LINK_LIBRARIES(tsgraphics tscore)

copy_source_groups(${src_list})