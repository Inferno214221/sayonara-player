project(sayonara_components)

set(CMAKE_AUTOUIC OFF)

file(GLOB_RECURSE SOURCES Bookmarks *.cpp)
file(GLOB_RECURSE SOURCES Broadcasting *.cpp)
file(GLOB_RECURSE SOURCES Converter *.cpp)
file(GLOB_RECURSE SOURCES Covers *.cpp)
file(GLOB_RECURSE SOURCES Directories *.cpp)
file(GLOB_RECURSE SOURCES Engine *.cpp)
file(GLOB_RECURSE SOURCES Library *.cpp)
file(GLOB_RECURSE SOURCES Lyrics *.cpp)
file(GLOB_RECURSE SOURCES MetaDataInfo *.cpp)
file(GLOB_RECURSE SOURCES Playlist *.cpp)
file(GLOB_RECURSE SOURCES PlaylistChooser *.cpp)
file(GLOB_RECURSE SOURCES PlayManager *.cpp)
file(GLOB_RECURSE SOURCES Preferences *.cpp)
file(GLOB_RECURSE SOURCES RemoteControl *.cpp)
file(GLOB_RECURSE SOURCES Session *.cpp)
file(GLOB_RECURSE SOURCES Shutdown *.cpp)
file(GLOB_RECURSE SOURCES Streaming *.cpp)
file(GLOB_RECURSE SOURCES Tagging *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_subdirectory(DBus)

target_link_libraries(${PROJECT_NAME}
	say_db
	say_interfaces_raw_sound_receiver
	interface_notifications
	comp_dbus

	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_BASE_APP_LIBRARIES}

	Qt5::DBus
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

