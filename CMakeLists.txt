# CMakeLists.txt for webserver
#
# created 2018 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This webserver)
project(${This} C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)

enable_testing()

get_directory_property(ParentDirectory PARENT_DIRECTORY)
if(ParentDirectory STREQUAL "")

    # Organize applications, libraries, etc. in folders in the IDE.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Ensure proper loading of side-loaded shared libraries
    # on all targets.
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # Don't add the goofy "d" postfix for debug builds of libraries.
    set(CMAKE_DEBUG_POSTFIX "")

    # Google Test
    set(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)
    add_subdirectory(googletest)
    set_target_properties(gtest gtest_main PROPERTIES
        FOLDER Libraries
    )

    # All other libraries can be pulled in without further configuration.
    add_subdirectory(Reactor)
    add_subdirectory(Protocol)
    add_subdirectory(TestDemo)
endif(ParentDirectory STREQUAL "")

# Add subdirectories directly in this repository.

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
