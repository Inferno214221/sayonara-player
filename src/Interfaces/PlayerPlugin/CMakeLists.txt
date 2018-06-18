project(say_interfaces_player_plugin)

set(SOURCES
	PlayerPlugin.cpp
	PlayerPluginHandler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	say_gui_utils
)

if( WITH_COTIRE )
    cotire(${PROJECT_NAME})
endif()

