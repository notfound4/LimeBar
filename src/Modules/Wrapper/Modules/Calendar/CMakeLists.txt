add_library(wrapper-calendar SHARED calendar_module.cpp)
target_link_libraries(wrapper-calendar limebar-wrapper)

install(TARGETS wrapper-calendar
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/LimeBar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)