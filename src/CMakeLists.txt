add_library(limebar SHARED Resource.cpp CustomOptionDescription.cpp OptionPrinter.cpp Bar.cpp Monitor.cpp Area.cpp)

add_executable(limebar-bin limebar.cpp)
target_link_libraries(limebar-bin ${XCB_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${Boost_LIBRARIES} ${GDK-PixBuf_LIBRARIES} limebar)
set_target_properties(limebar-bin PROPERTIES OUTPUT_NAME limebar)

add_executable(list_fonts example1.c)
target_link_libraries(list_fonts ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES})

# Install
install(TARGETS limebar-bin
	DESTINATION ${CMAKE_INSTALL_BINDIR}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install(TARGETS limebar
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)