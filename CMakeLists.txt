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

set(VAR_SAYONARA_VERSION "1.1.0")

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

if(${UNIX})
	execute_process(COMMAND update-menus)
endif(${UNIX})


set(LANGUAGE_PACK_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/language-pack)

file(GLOB_RECURSE HEADER_FILES ./src *.h)
file(GLOB_RECURSE SOURCE_FILES ./src. *.cpp)
file(GLOB_RECURSE LANG_FILES ./src *.ts)

add_custom_target(language-pack
	COMMAND ${CMAKE_COMMAND} -E make_directory ${LANGUAGE_PACK_DIRECTORY}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${LANGUAGE_PACK_DIRECTORY}/sources
	COMMAND ${CMAKE_COMMAND} -E make_directory ${LANGUAGE_PACK_DIRECTORY}/languages

	COMMAND ${CMAKE_COMMAND} -E copy ${HEADER_FILES} ${LANGUAGE_PACK_DIRECTORY}/sources
	COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILES} ${LANGUAGE_PACK_DIRECTORY}/sources
	COMMAND ${CMAKE_COMMAND} -E copy ${LANG_FILES} ${LANGUAGE_PACK_DIRECTORY}/languages
	DEPENDS sayonara
)

