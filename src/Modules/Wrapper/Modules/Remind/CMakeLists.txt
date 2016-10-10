add_library(wrapper-remind SHARED remind_module.cpp Remind_Thread.cpp)
target_link_libraries(wrapper-remind limebar-wrapper limebar-remind)

install(TARGETS wrapper-remind
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/LimeBar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)