
SET(CTGraphics_list
	graphicscore.cpp
	graphicseffect.cpp
	graphicsmodel.cpp
	graphicsqueue.cpp
	graphicstext.cpp
	graphicstexture.cpp
	pch.cpp
	pch.h
)

ADD_LIBRARY(
	CTGraphics SHARED
	${CTGraphics_list}
)

TARGET_LINK_LIBRARIES(CTGraphics CTCore)
