add_library(wrapper-portage SHARED portage_module.cpp)
target_link_libraries(wrapper-portage limebar-wrapper)

install(TARGETS wrapper-portage
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/LimeBar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)