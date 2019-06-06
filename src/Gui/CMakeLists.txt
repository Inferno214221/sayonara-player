project(sayonara_gui)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)

add_subdirectory(Preferences)
add_subdirectory(Plugins)
add_subdirectory(Resources)
add_subdirectory(Utils)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

add_library(${PROJECT_NAME} "")
add_subdirectory(Covers)
add_subdirectory(Directories)
add_subdirectory(ImportDialog)
add_subdirectory(InfoDialog)
add_subdirectory(Library)
add_subdirectory(Player)
add_subdirectory(Playlist)
add_subdirectory(Shutdown)
add_subdirectory(TagEdit)


target_link_libraries(${PROJECT_NAME}
	gui_plugins
	gui_preferences

	say_interfaces_library_container
	say_interfaces_notifications

	Qt5::DBus
)
