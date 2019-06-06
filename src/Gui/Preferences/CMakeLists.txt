project(gui_preferences)

file(GLOB_RECURSE SOURCES . *.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	gui_utils
	say_interfaces_notifications
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

