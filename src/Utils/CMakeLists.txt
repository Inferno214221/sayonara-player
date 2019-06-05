project(say_utils)

set(CMAKE_AUTOMOC OFF)

qt5_wrap_cpp(MOC_FILES
	WebAccess/AsyncWebAccess.h
	WebAccess/IcyWebAccess.h
	WebAccess/Proxy.h
	Language/Language.h
	Parser/StreamParser.h
	Settings/SettingNotifier.h
)


pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})

include_directories(${TAGLIB_INCLUDE_DIRS})
link_directories(${TAGLIB_LIBRARY_DIRS})

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${MOC_FILES})

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
