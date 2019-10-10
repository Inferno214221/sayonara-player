project(sayonara_application)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR},
	${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCES . *.cpp)
file(GLOB_RECURSE HEADERS . *.h)

set(APPLICATION_LIBRARIES
	gui_player
	gui_library
	gui_directories
	gui_plugins
	gui_preferences
	gui_somafm
	gui_soundcloud
	gui_resources

	sayonara_components
	interface_library
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}
	${APPLICATION_LIBRARIES}
	sayonara_dbus
	sayonara_utils
)

