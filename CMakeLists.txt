cmake_minimum_required(VERSION 3.7)
project("Sayonara Player")

option(WITH_DOC "Building with doxygen support" OFF)
option(WITH_TESTS "Building with tests" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
set(VAR_SAYONARA_VERSION "1.5.2-beta3")

set(SAYONARA_VERSION "${VAR_SAYONARA_VERSION}")
set(SAYONARA_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}")
set(SAYONARA_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(SAYONARA_INSTALL_LIB_PATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/sayonara/")
set(SAYONARA_INSTALL_BIN_PATH "${CMAKE_INSTALL_PREFIX}/bin/")
set(SAYONARA_INSTALL_SHARE_PATH "${CMAKE_INSTALL_PREFIX}/share/sayonara/")

message("Build Sayonara ${VAR_SAYONARA_VERSION}")
message("Will install to ${CMAKE_INSTALL_PREFIX}")

# used for cmakedefine in Macros.h.in
set(SAYONARA_WITH_TESTS ${WITH_TESTS})
string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWER_CMAKE_BUILD_TYPE)
if("${LOWER_CMAKE_BUILD_TYPE}" MATCHES "debug")
	set(SAYONARA_WITH_TESTS 1)
endif()

add_subdirectory(src)
add_subdirectory(resources)

message("Lower cmake build type ${LOWER_CMAKE_BUILD_TYPE}")

if( SAYONARA_WITH_TESTS )
	message("Compiling with tests...")
	enable_testing()
	add_subdirectory(test)
else()
	message("Compiling without tests...")
	message(" In order to change that, use -DWITH_TESTS=1")
endif()

set(CMAKE_SKIP_RPATH:BOOL ON)
set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")

message ("Looking for doxygen")
find_package(Doxygen)

if (WITH_DOC)
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


