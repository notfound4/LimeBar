MACRO(SUBDIRADD curdir)
  FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${curdir} ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/*)
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${curdir}/${child})
			option(BUILD_${child} "Build the module" ON)
			if(${BUILD_${child}})
				add_subdirectory(${curdir}/${child})
      endif()
    ENDIF()
  ENDFOREACH()
ENDMACRO()