project(say_gui_plugins)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_comp_bookmarks
	say_comp_broadcasting
	say_comp_engine
	say_comp_playlist_chooser
	say_comp_streamplugins
	say_comp_converter
	say_gui_utils

	say_interfaces_player_plugin
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()
