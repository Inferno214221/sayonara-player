project(say_utils)

include(CheckIncludeFileCXX)
check_include_file_cxx("cxxabi.h" HAVE_CXX_ABI)
if(${HAVE_CXX_ABI})
	add_definitions("-DHAVE_CXX_ABI")
	message("Compile with demangle")
else()
	message("Demangle not found")
endif()

pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})

include_directories(${TAGLIB_INCLUDE_DIRS})
link_directories(${TAGLIB_LIBRARY_DIRS})

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Network
	Qt5::Xml
	Qt5::DBus
	Qt5::Gui
	tag
	${ZLIB_LIBRARIES}
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
