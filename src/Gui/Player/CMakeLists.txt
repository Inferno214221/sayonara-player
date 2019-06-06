project(gui_player)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})

#Covers, InfoDialog, Playlist, Plugins, Shutdown, Utils
target_link_libraries(${PROJECT_NAME}
	gui_playlist
	gui_covers
	say_interfaces_library_container
)
