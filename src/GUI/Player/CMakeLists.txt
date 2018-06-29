project(say_gui_player)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE RAW_UI_FILES . *.ui)

qt5_wrap_ui(UI_FILES ${RAW_UI_FILES})

set(PLAYER_LIBRARIES

	say_gui_playlist
	say_interfaces_player_plugin
	say_interfaces_notifications
	say_gui_plugins
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${UI_FILES})
target_link_libraries(${PROJECT_NAME}
	${PLAYER_LIBRARIES}
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

