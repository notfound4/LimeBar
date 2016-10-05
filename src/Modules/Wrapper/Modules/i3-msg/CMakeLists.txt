add_library(wrapper-i3-msg SHARED i3msg_module.cpp)
target_link_libraries(wrapper-i3-msg limebar-wrapper)

install(TARGETS wrapper-i3-msg
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/LimeBar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)