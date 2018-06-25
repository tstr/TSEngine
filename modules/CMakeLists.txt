################################################################################################
#	Engine modules
################################################################################################

include(AddModule)

ADD_SUBDIRECTORY(tscore)
ADD_SUBDIRECTORY(tsengine)
ADD_SUBDIRECTORY(tsgraphics)
ADD_SUBDIRECTORY(tsdx11)

################################################################################################

# Import file
# install(
    # EXPORT ${MODULE_EXPORT_GROUP}
    # DESTINATION "lib/cmake"
# )
