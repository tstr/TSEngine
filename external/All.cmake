################################################################################################
#	External dependencies
################################################################################################

include(ExternalProject)

# Current directory
set(EP_DIR "${CMAKE_CURRENT_LIST_DIR}")
# Prefix for external project
set(EP_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external")
# Intermediate installation directory for external projects
set(EP_STAGE_DIR "${EP_PREFIX}/stage")

# Set up external projects
include(${EP_DIR}/ExternalAssimp.cmake)

# Forward installation of external projects to actual installation directory
install(DIRECTORY ${EP_STAGE_DIR}/ DESTINATION ".")

################################################################################################
