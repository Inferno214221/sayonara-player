project(gui_player)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

#Covers, InfoDialog, Playlist, Plugins, Shutdown, Utils
target_link_libraries(${PROJECT_NAME}
	gui_playlist
	gui_covers
	gui_infodialog
	interface_notifications
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

