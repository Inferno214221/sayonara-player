project(say_application)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR},
	${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCES . *.cpp)

set(APPLICATION_LIBRARIES
	say_gui_library
	say_gui_directory_widget

	say_gui_player
	say_gui_preferences
	say_gui_plugins

	say_interfaces_libraryinterface
	say_comp_remote_control
	say_comp_session
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
	say_gui_resources
)



