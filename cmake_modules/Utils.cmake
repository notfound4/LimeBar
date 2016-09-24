MACRO(SUBDIRADD curdir)
  	FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${curdir} ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/*)
  	FOREACH(child ${children})
    	IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
			option(BUILD_${child} "Build the module" ON)
			if(${BUILD_${child}})
  				FILE(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child}/*.cpp)
  				add_library(limebar-${child} SHARED ${files})
  				install(TARGETS limebar-${child}
					DESTINATION ${CMAKE_INSTALL_LIBDIR}/LimeBar
					PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
				)
				add_subdirectory(${curdir}/${child})
      		endif()
    	ENDIF()
  	ENDFOREACH()
ENDMACRO()