#
#	The default InstallRequiredSystemLibraries module doesn't work for visual studio 2015 so we have to use this
#

set(TS_SYSTEM_LIB_TEMPDIR ${PROJECT_BINARY_DIR}/systemlibs/)

#if (FALSE)
if(DEFINED MSVC_VERSION AND NOT MSVC_VERSION LESS 1900)
	
	# Internal: Architecture-appropriate library directory names.
	if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "ARM")
		set(_winsdk_arch8 arm) # what the WDK for Win8+ calls this architecture
	else()
		if(CMAKE_SIZEOF_VOID_P MATCHES "8")
			set(_winsdk_arch8 x64) # what the WDK for Win8+ calls this architecture
		else()
			set(_winsdk_arch8 x86) # what the WDK for Win8+ calls this architecture
		endif()
	endif()

	# The CRT is distributed with MSVS.
	get_filename_component(MSVS_DIR
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0;ShellFolder]" ABSOLUTE)

	# As of MSVC 19 the CRT depends on the 'univeral' CRT (which is part of Windows development kit 10 and above).
	# http://blogs.msdn.com/b/vcblog/archive/2015/03/03/introducing-the-universal-crt.aspx [^]
	get_filename_component(WINDOWS_KIT_DIR
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)

	file(GLOB CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_DEBUG
		"${MSVS_DIR}/VC/redist/debug_nonredist/${_winsdk_arch8}/Microsoft.VC140.DebugCRT/*.dll"
		#"${WINDOWS_KIT_DIR}/Redist/ucrt/DLLs/${_winsdk_arch8}/api-ms-win-*.dll"
		"${WINDOWS_KIT_DIR}/bin/${_winsdk_arch8}/ucrt/*.dll"
	)
	
	file(GLOB CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_RELEASE
		"${MSVS_DIR}/VC/redist/${_winsdk_arch8}/Microsoft.VC140.CRT/*.dll"
		#"${WINDOWS_KIT_DIR}/Redist/ucrt/DLLs/${_winsdk_arch8}/*.dll"
	)
	
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_RELEASE})
	
else()

	set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS 1)
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${TS_SYSTEM_LIB_TEMPDIR})
	set(CMAKE_INSTALL_DEBUG_LIBRARIES 0)
	include(InstallRequiredSystemLibraries)

endif()


FILE(COPY ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${TS_SYSTEM_LIB_TEMPDIR})

ADD_CUSTOM_TARGET(
	InstallSystemLibs ALL
)

ADD_CUSTOM_COMMAND(
	TARGET InstallSystemLibs
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${TS_SYSTEM_LIB_TEMPDIR}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/"
)

SET_TARGET_PROPERTIES(InstallSystemLibs PROPERTIES FOLDER CMakeCustomTargets)
