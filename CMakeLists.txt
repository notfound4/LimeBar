# Version minimum de cmake
cmake_minimum_required (VERSION 2.8)
# Nom du projet
project (LimeBar)

# Set build options
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release"
    	CACHE STRING "Choose the type of build, options are: Debug Release."
    	FORCE
	)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# Option de compilation pour les diverses types de compilation
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -pedantic -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

# Executable dans le répertoire Exec
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Add modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules)
include(GetGitRevisionDescription)
include(Pod2Man)

# Get git description
git_describe(GIT_DESC "--tags")

# find packages
find_package(XCB COMPONENTS xcb xcb-randr REQUIRED)
include_directories(${XCB_INCLUDE_DIRS})
find_package(Cairo REQUIRED)
include_directories(${CAIRO_INCLUDE_DIRS})
find_package(Pango REQUIRED)
include_directories(${PANGO_INCLUDE_DIRS})
find_package( Boost COMPONENTS program_options filesystem system regex REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
find_package( GDK-PixBuf REQUIRED )
include_directories( ${GDK-PixBuf_INCLUDE_DIRS} )

# Add documentation
add_manpage_target()
pod2man(README.pod limebar 1)

include_directories(include)
add_subdirectory(src)
add_subdirectory(include)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)