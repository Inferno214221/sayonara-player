project(gui_preferences)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	gui_utils
	interface_notifications
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

