add_library(limebar SHARED CustomOptionDescription.cpp OptionPrinter.cpp FontHandler.cpp Bar.cpp ColorHandler.cpp Types.cpp XHandler.cpp Monitor.cpp Area.cpp DatabaseHelper.cpp)

add_executable(limebar-bin limebar.cpp)
target_link_libraries(limebar-bin ${XCB_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGO_LIBRARIES} ${Boost_LIBRARIES} ${GDK-PixBuf_LIBRARIES} limebar)
set_target_properties(limebar-bin PROPERTIES OUTPUT_NAME limebar)

# Install
install(TARGETS limebar-bin
	DESTINATION ${CMAKE_INSTALL_BINDIR}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install(TARGETS limebar
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/LimeBar
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)