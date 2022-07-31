cmake_minimum_required(VERSION 3.8)
project("Sayonara Player")

option(WITH_DOC "Building with doxygen support" OFF)
option(WITH_TESTS "Building with tests" OFF)
option(WITH_COTIRE "Building with cotire PCH and compile time optimizations" OFF)
option(LINK_GSTREAMER_PLUGINS "Link the most common GStreamer plugins into the binary. Used for the creation of AppImage files")
option(WITH_CCACHE "Use CCache if available" OFF)
option(DISABLE_NATIVE_DIR_DIALOGS "Disable native directory dialogs (problem for snap and flatpak)" OFF)

get_property(HAS_64_SUFFIX GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
if(HAS_64_SUFFIX)
	set(LIB_SUFFIX "64")
else()
	set(LIB_SUFFIX "")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

set(SAYONARA_VERSION "1.8.0-beta1")
set(SAYONARA_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}")
set(SAYONARA_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(SAYONARA_INSTALL_BIN_PATH "${CMAKE_INSTALL_PREFIX}/bin/")
set(SAYONARA_INSTALL_LIB_PATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/sayonara/")
set(SAYONARA_INSTALL_SHARE_PATH "${CMAKE_INSTALL_PREFIX}/share/sayonara/")

message("Build Sayonara ${VAR_SAYONARA_VERSION}")
message("Will install to ${CMAKE_INSTALL_PREFIX}")

# used for cmakedefine in Macros.h.in
set(SAYONARA_WITH_TESTS ${WITH_TESTS})
string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWER_CMAKE_BUILD_TYPE)
if("${LOWER_CMAKE_BUILD_TYPE}" MATCHES "debug")
	set(SAYONARA_WITH_TESTS 1)
endif()

add_subdirectory(dist)
add_subdirectory(src)
add_subdirectory(resources)

message("Lower cmake build type ${LOWER_CMAKE_BUILD_TYPE}")

if(SAYONARA_WITH_TESTS)
	message("Compiling with tests...")
	enable_testing()
	add_subdirectory(test)
else()
	message("Compiling without tests...")
	message(" In order to change that, use -DWITH_TESTS=1")
endif()

set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")

# doxygen

if (WITH_DOC)
	message ("Looking for doxygen")
	find_package(Doxygen)

	if (DOXYGEN_FOUND)
		message ("Doxygen found")
		set(DOXYFILE_IN "${CMAKE_SOURCE_DIR}/docs/doxygen.in")
		set(DOXYFILE_OUT "${CMAKE_BINARY_DIR}/docs/doxygen.cfg")
		set(DOXYFILE_SRC_DIR "${CMAKE_SOURCE_DIR}")
		set(DOXYFILE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")

		include(FindPerl OPTIONAL)
		set(DOXYFILE_PERL_EXECUTABLE "${PERL_EXECUTABLE}")

		configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

		add_custom_target(doc
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src"
			COMMENT "Generating documentation..."
			VERBATIM
		)

	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html DESTINATION share/doc/sayonara/doxygen)

	endif (DOXYGEN_FOUND)
endif (WITH_DOC)

