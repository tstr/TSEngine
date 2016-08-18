#TSGRAPHICS

SET(src_list
	colour.h
	rendermodule.h
	rendermodule.cpp
	rendermoduleloader.cpp
	renderapi.h
	api/DX11/DX11base.h
	api/DX11/DX11render.h
	api/DX11/DX11render.cpp
	api/DX11/DX11adapter.cpp
)

ADD_LIBRARY(
	tsgraphics STATIC
	${src_list}
)


TARGET_LINK_LIBRARIES(tsgraphics tscore)

copy_source_groups(${src_list})