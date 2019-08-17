project(sayonara_utils)

set(CMAKE_AUTOMOC OFF)

qt5_wrap_cpp(MOC_FILES
	WebAccess/AsyncWebAccess.h
	WebAccess/IcyWebAccess.h
	WebAccess/Proxy.h
	Language/Language.h
	Parser/StreamParser.h
	Settings/SettingNotifier.h
)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${MOC_FILES})
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Network
	Qt5::Xml
	Qt5::Gui
	Qt5::DBus

	${TAGLIB_LIBRARIES}
	${ZLIB_LIBRARIES}
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
