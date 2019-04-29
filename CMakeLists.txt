cmake_minimum_required(VERSION 2.8)
project("Sayonara Player")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

if( WITH_COTIRE )
	include(cotire)
	message("Compiling with Cotire")
	add_definitions(-D__STRICT_ANSI__)
else()
	message("Compiling without Cotire")
endif()

set(VAR_SAYONARA_VERSION "1.3.0-stable3")

set(SAYONARA_VERSION "${VAR_SAYONARA_VERSION}")
set(SAYONARA_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}")
set(SAYONARA_INSTALL_LIB_PATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/sayonara/")
set(SAYONARA_INSTALL_BIN_PATH "${CMAKE_INSTALL_PREFIX}/bin/")
set(SAYONARA_INSTALL_SHARE_PATH "${CMAKE_INSTALL_PREFIX}/share/sayonara/")

configure_file(Macros.h.in "${CMAKE_CURRENT_BINARY_DIR}/src/Utils/Macros.h" @ONLY)

message("Build Sayonara ${VAR_SAYONARA_VERSION}")
message("Will install to ${CMAKE_INSTALL_PREFIX}")

add_subdirectory(src)
add_subdirectory(resources)

string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWER_CMAKE_BUILD_TYPE)
message("Lower cmake build type ${LOWER_CMAKE_BUILD_TYPE}")
if("${LOWER_CMAKE_BUILD_TYPE}" MATCHES "debug")
	SET(WITH_TESTS 1)
endif()

if( WITH_TESTS )
	message("Compiling with tests...")
	enable_testing()
	add_subdirectory(test)
else()
	message("Compiling without tests...")
	message(" In order to change that, use -DWITH_TESTS=1")
endif()

set(CMAKE_SKIP_RPATH:BOOL ON)
set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")


