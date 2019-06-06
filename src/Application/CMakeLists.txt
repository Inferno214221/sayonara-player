project(say_application)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR},
	${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCES . *.cpp)

set(APPLICATION_LIBRARIES
	gui_player
	gui_library
	gui_directories
	gui_plugins
	gui_preferences

	say_interfaces_libraryinterface
	sayonara_components
)

if(NOT WIN32 OR NOT DEFINED WIN32)
	set(APPLICATION_LIBRARIES
	${APPLICATION_LIBRARIES}
	say_comp_dbus
	)
else()
	set(APPLICATION_LIBRARIES
		${APPLICATION_LIBRARIES}
		sayonara_somafm
		sayonara_soundcloud
	)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
	${APPLICATION_LIBRARIES}
)

add_library(sayonara_test SHARED ${SOURCES})
target_link_libraries(sayonara_test
	${APPLICATION_LIBRARIES}
	gui_resources
)



