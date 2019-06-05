project(sayonara_gui)

set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

add_subdirectory(Covers)
add_subdirectory(Directories)
add_subdirectory(ImportDialog)
add_subdirectory(InfoDialog)
add_subdirectory(Library)
add_subdirectory(Player)
add_subdirectory(Playlist)
add_subdirectory(Plugins)
add_subdirectory(Preferences)
add_subdirectory(Resources)
add_subdirectory(Shutdown)
add_subdirectory(TagEdit)
add_subdirectory(Utils)


target_link_libraries(${PROJECT_NAME}
	sayonara_components

	say_interfaces_library_container
	say_interfaces_notifications

	say_utils

	gui_covers
	gui_directories
	gui_importdialog
	gui_infodialog
	gui_library
	gui_player
	gui_playlist
	gui_plugins
	gui_preferences
	gui_resources
	gui_shutdown
	gui_tagedit
	gui_utils

	Qt5::Widgets
	Qt5::DBus
)
